#include "storage/table_heap.h"

bool TableHeap::InsertTuple(Row &row, Transaction *txn) {
  // 往现有的页中插入
  TablePage *page;
  for (page_id_t page_id = first_page_id_; page_id != INVALID_PAGE_ID;) {
    page = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(page_id));
    if (page->InsertTuple(row, schema_, txn, lock_manager_, log_manager_)) {
      buffer_pool_manager_->UnpinPage(page->GetPageId(), true);
      return true;
    }
    page_id = page->GetNextPageId();
    buffer_pool_manager_->UnpinPage(page->GetPageId(), false);
  }
  // 现有的页插入失败 新建一个页
  page_id_t new_page_id, curr_page_id;
  buffer_pool_manager_->NewPage(new_page_id);
  // 更新现有最后一页的信息
  curr_page_id = page->GetPageId();
  page->SetNextPageId(new_page_id);
  buffer_pool_manager_->FlushPage(page->GetPageId());
  // 打开新页并初始化
  page = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(new_page_id));
  page->Init(new_page_id, curr_page_id, log_manager_, txn);
  if (page->InsertTuple(row, schema_, txn, lock_manager_, log_manager_)) {
    buffer_pool_manager_->UnpinPage(page->GetPageId(), true);
    buffer_pool_manager_->UnpinPage(page->GetPageId(), true);
    return true;
  }
  buffer_pool_manager_->UnpinPage(page->GetPageId(), true);
  buffer_pool_manager_->UnpinPage(page->GetPageId(), true);
  return false;
}

bool TableHeap::MarkDelete(const RowId &rid, Transaction *txn) {
  // Find the page which contains the tuple.
  auto page = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(rid.GetPageId()));
  // If the page could not be found, then abort the transaction.
  if (page == nullptr) {
    return false;
  }
  // Otherwise, mark the tuple as deleted.
  page->WLatch();
  page->MarkDelete(rid, txn, lock_manager_, log_manager_);
  page->WUnlatch();
  buffer_pool_manager_->UnpinPage(page->GetTablePageId(), true);
  return true;
}

// 原先new_row的类型是const引用，但是如果没有理解错的话new_row.rid_的值可能会变化
// 所以这里将const删去了，相应的对rid_有更新
bool TableHeap::UpdateTuple(Row &new_row, const RowId &rid, Transaction *txn) {
  auto page = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(rid.GetPageId()));
  Row old_row(rid);
  page->GetTuple(&old_row, schema_, txn, lock_manager_);
  int res = page->UpdateTuple(new_row, &old_row, schema_, txn, lock_manager_, log_manager_);
  if (res == 0) {
    // no enough space to update, delete and insert;
    // todo 这里mark完要不要apply？
    MarkDelete(rid, txn);
    ApplyDelete(rid, txn);
    InsertTuple(new_row, txn);
    buffer_pool_manager_->UnpinPage(page->GetPageId(), true);
    return true;
  } else if (res == 1) {
    // successfully update
    new_row.SetRowId(rid);
    buffer_pool_manager_->UnpinPage(page->GetPageId(), true);
    return true;
  } else {
    buffer_pool_manager_->UnpinPage(page->GetPageId(), false);
    return false;
  }
}

void TableHeap::ApplyDelete(const RowId &rid, Transaction *txn) {
  // Step1: Find the page which contains the tuple.
  // Step2: Delete the tuple from the page.
  auto page = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(rid.GetPageId()));
  page->ApplyDelete(rid, txn, log_manager_);
  buffer_pool_manager_->UnpinPage(page->GetPageId(), true);
}

void TableHeap::RollbackDelete(const RowId &rid, Transaction *txn) {
  // Find the page which contains the tuple.
  auto page = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(rid.GetPageId()));
  assert(page != nullptr);
  // Rollback the delete.
  page->WLatch();
  page->RollbackDelete(rid, txn, log_manager_);
  page->WUnlatch();
  buffer_pool_manager_->UnpinPage(page->GetTablePageId(), true);
}

// todo 现在释放页的方式对吗? 以及怎么销毁TableHeap？ 待测试
void TableHeap::FreeHeap() {
  for (page_id_t page_id = first_page_id_; page_id != INVALID_PAGE_ID;) {
    auto page = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(page_id));
    page_id = page->GetNextPageId();
    buffer_pool_manager_->DeletePage(page->GetPageId());
  }
  first_page_id_ = INVALID_PAGE_ID;
}

bool TableHeap::GetTuple(Row *row, Transaction *txn) {
  RowId rid = row->GetRowId();
  auto page = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(rid.GetPageId()));
  if (page->GetTuple(row, schema_, txn, lock_manager_)) {
    buffer_pool_manager_->UnpinPage(page->GetPageId(), false);
    return true;
  }
  buffer_pool_manager_->UnpinPage(page->GetPageId(), false);
  return false;
}

TableIterator TableHeap::Begin(Transaction *txn) {
  auto page = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(GetFirstPageId()));
  RowId row_id;
  page->GetFirstTupleRid(&row_id);
  Row row(row_id);
  if (!(row_id==INVALID_ROWID)) {
    page->GetTuple(&row, this->schema_, txn, this->lock_manager_);
  }
  buffer_pool_manager_->UnpinPage(page->GetPageId(), false);
  return TableIterator(this, row);
}

TableIterator TableHeap::End() { return TableIterator(this, Row(INVALID_ROWID)); }
