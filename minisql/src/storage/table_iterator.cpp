#include "storage/table_iterator.h"
#include "common/macros.h"
#include "storage/table_heap.h"

TableIterator::TableIterator() {
  table_heap_ = nullptr;
  row_ = nullptr;
}

TableIterator::TableIterator(TableHeap *table_heap, const Row &row) {
  table_heap_ = table_heap;
  row_ = new Row(row);
}

TableIterator::TableIterator(const TableIterator &other) {
  table_heap_ = other.table_heap_;
  row_ = other.row_;
}

TableIterator::~TableIterator() = default;

const Row &TableIterator::operator*() { return *row_; }

Row *TableIterator::operator->() { return row_; }

TableIterator &TableIterator::operator++() {
  page_id_t next_page_id;
  RowId next_row_id;
  TablePage *page =
      reinterpret_cast<TablePage *>(table_heap_->buffer_pool_manager_->FetchPage(row_->GetRowId().GetPageId()));

  // 当前页没有next row
  if (!page->GetNextTupleRid(row_->GetRowId(), &next_row_id)) {
    do {
      // 尝试开新页
      next_page_id = page->GetNextPageId();
      // 没有新页了
      if (next_page_id == INVALID_PAGE_ID) {
        row_->SetRowId(INVALID_ROWID);
        break;
      }
      // 还有新页接着找
      table_heap_->buffer_pool_manager_->UnpinPage(page->GetPageId(), false);
      page = reinterpret_cast<TablePage *>(table_heap_->buffer_pool_manager_->FetchPage(next_page_id));
    } while (!page->GetFirstTupleRid(&next_row_id));
  }

  table_heap_->buffer_pool_manager_->UnpinPage(page->GetPageId(), false);
  row_->SetRowId(next_row_id);
  if (!(next_row_id == INVALID_ROWID))
    page->GetTuple(row_, table_heap_->schema_, nullptr, table_heap_->lock_manager_);
  return *this;
}

TableIterator TableIterator::operator++(int) {
  TableIterator res = *this;
  page_id_t next_page_id;
  RowId next_row_id;
  TablePage *page =
      reinterpret_cast<TablePage *>(table_heap_->buffer_pool_manager_->FetchPage(row_->GetRowId().GetPageId()));

  // 如果有next_tuple 直接赋给iter然后返回
  if (!page->GetNextTupleRid(row_->GetRowId(), &next_row_id)) {
    // 没有next_tuple 但是可能有next_page 尝试换页
    do {
      next_page_id = page->GetNextPageId();
      if (next_page_id == INVALID_PAGE_ID) {
        row_->SetRowId(INVALID_ROWID);
        break;
      }
      table_heap_->buffer_pool_manager_->UnpinPage(page->GetPageId(), false);
      page = reinterpret_cast<TablePage *>(table_heap_->buffer_pool_manager_->FetchPage(next_page_id));
    } while (!page->GetFirstTupleRid(&next_row_id));
  }

  table_heap_->buffer_pool_manager_->UnpinPage(page->GetPageId(), false);
  row_->SetRowId(next_row_id);
  if (!(next_row_id == INVALID_ROWID))
    page->GetTuple(row_, table_heap_->schema_, nullptr, table_heap_->lock_manager_);
  return res;
}
int64_t TableIterator::GetRowId_64() { return row_->GetRowId().Get(); }
