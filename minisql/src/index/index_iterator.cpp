#include "index/index_iterator.h"
#include "index/basic_comparator.h"
#include "index/generic_key.h"

INDEX_TEMPLATE_ARGUMENTS INDEXITERATOR_TYPE::IndexIterator() {
  leaf_page_ = nullptr;
  buffer_pool_manager_ = nullptr;
  index_ = INVALID_INDEX;
}

INDEX_TEMPLATE_ARGUMENTS INDEXITERATOR_TYPE::IndexIterator(
    BPlusTreeLeafPage<KeyType, ValueType, KeyComparator> *leaf_page, BufferPoolManager *buffer_pool_manager,
    int index) {
  leaf_page_ = leaf_page;
  buffer_pool_manager_ = buffer_pool_manager;
  index_ = index;
}

INDEX_TEMPLATE_ARGUMENTS INDEXITERATOR_TYPE::IndexIterator(
    const IndexIterator<KeyType, ValueType, KeyComparator> &itr) {
  leaf_page_ = itr.leaf_page_;
  index_ = itr.index_;
  buffer_pool_manager_ = itr.buffer_pool_manager_;
}

INDEX_TEMPLATE_ARGUMENTS INDEXITERATOR_TYPE::~IndexIterator() {
  // 似乎不需要
}

INDEX_TEMPLATE_ARGUMENTS const MappingType &INDEXITERATOR_TYPE::operator*() { return leaf_page_->GetItem(index_); }

// todo 待测试
INDEX_TEMPLATE_ARGUMENTS INDEXITERATOR_TYPE &INDEXITERATOR_TYPE::operator++() {
  ++index_;
  if (index_ >= leaf_page_->GetSize()) {
    page_id_t next_page_id = leaf_page_->GetNextPageId();
    // 没有下一页了
    if (next_page_id == INVALID_PAGE_ID) {
      index_ = INVALID_INDEX;
    }
    // 还有下一页
    else {
      auto page = buffer_pool_manager_->FetchPage(next_page_id);
      leaf_page_ = reinterpret_cast<BPlusTreeLeafPage<KeyType, ValueType, KeyComparator> *>(page);
      index_ = 0;
      buffer_pool_manager_->UnpinPage(next_page_id, false);
    }
  }
  return *this;
}

INDEX_TEMPLATE_ARGUMENTS
bool INDEXITERATOR_TYPE::operator==(const IndexIterator &itr) const {
  return leaf_page_ == itr.leaf_page_ && index_ == itr.index_ && buffer_pool_manager_ == itr.buffer_pool_manager_;
}

INDEX_TEMPLATE_ARGUMENTS
bool INDEXITERATOR_TYPE::operator!=(const IndexIterator &itr) const { return !(*this == itr); }

template class IndexIterator<int, int, BasicComparator<int>>;

template class IndexIterator<GenericKey<4>, RowId, GenericComparator<4>>;

template class IndexIterator<GenericKey<8>, RowId, GenericComparator<8>>;

template class IndexIterator<GenericKey<16>, RowId, GenericComparator<16>>;

template class IndexIterator<GenericKey<32>, RowId, GenericComparator<32>>;

template class IndexIterator<GenericKey<64>, RowId, GenericComparator<64>>;
