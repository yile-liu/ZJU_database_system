#include "page/b_plus_tree_leaf_page.h"
#include "index/b_plus_tree.h"
#include <algorithm>
#include "index/basic_comparator.h"
#include "index/generic_key.h"

/*****************************************************************************
 * HELPER METHODS AND UTILITIES
 *****************************************************************************/

/**
 * Init method after creating a new leaf page
 * Including set page type, set current size to zero, set page id/parent id, set
 * next page id and set max size
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::Init(page_id_t page_id, page_id_t parent_id, int max_size) {
  SetPageType(IndexPageType::LEAF_PAGE);
  SetLSN(INVALID_LSN);  // no use here
  SetSize(0);
  SetMaxSize(max_size);
  SetParentPageId(parent_id);
  SetPageId(page_id);
}

/**
 * Helper methods to set/get next page id
 */
INDEX_TEMPLATE_ARGUMENTS
page_id_t B_PLUS_TREE_LEAF_PAGE_TYPE::GetNextPageId() const { return next_page_id_; }

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::SetNextPageId(page_id_t next_page_id) { next_page_id_ = next_page_id; }

/**
 * Helper method to find the first index i so that array_[i].first >= key
 * NOTE: This method is only used when generating index iterator
 */
INDEX_TEMPLATE_ARGUMENTS
int B_PLUS_TREE_LEAF_PAGE_TYPE::KeyIndex(const KeyType &key, const KeyComparator &comparator) const {
  assert(GetSize() >= 0);
  int st = 0, ed = GetSize() - 1;
  while (st <= ed) {  // find the last key in array <= input
    int mid = (ed - st) / 2 + st;
    if (comparator(array_[mid].first, key) >= 0)
      ed = mid - 1;
    else
      st = mid + 1;
  }
  return ed + 1;
}

/*
 * Helper method to find and return the key associated with input "index"(a.k.a
 * array offset)
 */
INDEX_TEMPLATE_ARGUMENTS
KeyType B_PLUS_TREE_LEAF_PAGE_TYPE::KeyAt(int index) const {
  // todo KeyAt(int index) 超出范围怎么处理？ 现在是直接访问对应下标不检查index范围
  // replace with your own code
  assert(index >= 0 && index < GetSize());
  KeyType key = array_[index].first;
  return key;
}

/*
 * Helper method to find and return the key & value pair associated with input
 * "index"(a.k.a array offset)
 */
INDEX_TEMPLATE_ARGUMENTS
const MappingType &B_PLUS_TREE_LEAF_PAGE_TYPE::GetItem(int index) {
  assert(index >= 0 && index < GetSize());
  return array_[index];
}

/*****************************************************************************
 * INSERTION
 *****************************************************************************/
/*
 * Insert key & value pair into leaf page ordered by key
 * @return page size after insertion
 */
INDEX_TEMPLATE_ARGUMENTS
int B_PLUS_TREE_LEAF_PAGE_TYPE::Insert(const KeyType &key, const ValueType &value, const KeyComparator &comparator) {
  assert(GetSize() < GetMaxSize() + 1);
  int targetIndex = KeyIndex(key, comparator);
  IncreaseSize(1);
  assert(targetIndex >= 0);
  for (int i = GetSize() - 2; i >= targetIndex; i--) {
    array_[i + 1].first = array_[i].first;
    array_[i + 1].second = array_[i].second;
  }

  array_[targetIndex].first = key;
  array_[targetIndex].second = value;
  return GetSize();
}

/*****************************************************************************
 * SPLIT
 *****************************************************************************/
/*
 * Remove half of key & value pairs from this page to "recipient" page
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::MoveHalfTo(BPlusTreeLeafPage *recipient) {
  assert(recipient != nullptr);
  int total = GetMaxSize() + 1;
  assert(GetSize() == total);
  //  copy last half
  int copy_index = (total) / 2;  // 7 is 4,5,6,7; 8 us 4,5,6,7,8
  for (int i = copy_index; i < total; ++i) {
    recipient->array_[i - copy_index].first = array_[i].first;
    recipient->array_[i - copy_index].second = array_[i].second;
  }
  //  set pointer
  recipient->SetNextPageId(GetNextPageId());
  SetNextPageId(recipient->GetPageId());
  //  set size ,is odd, bigger is last part;
  SetSize(copy_index);
  recipient->SetSize(total - copy_index);
}

/*
 * Copy starting from items, and copy {size} number of elements into me.
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::CopyNFrom(MappingType *items, int size) {
  // 复制元素
  std::copy(items, items + size, array_ + GetSize());
  // 更新this->size
  IncreaseSize(size);
}

/*****************************************************************************
 * LOOKUP
 *****************************************************************************/
/*
 * For the given key, check to see whether it exists in the leaf page. If it
 * does, then store its corresponding value in input "value" and return true.
 * If the key does not exist, then return false
 */
INDEX_TEMPLATE_ARGUMENTS
bool B_PLUS_TREE_LEAF_PAGE_TYPE::Lookup(const KeyType &key, ValueType &value, const KeyComparator &comparator) const {
  int idx = KeyIndex(key, comparator);
  if (idx < GetSize() && comparator(array_[idx].first, key) == 0) {
    value = array_[idx].second;
    return true;
  }
  return false;
}

/*****************************************************************************
 * REMOVE
 *****************************************************************************/
/*
 * First look through leaf page to see whether delete key exist or not. If
 * exist, perform deletion, otherwise return immediately.
 * NOTE: store key&value pair continuously after deletion
 * @return  page size after deletion
 */
INDEX_TEMPLATE_ARGUMENTS
int B_PLUS_TREE_LEAF_PAGE_TYPE::RemoveAndDeleteRecord(const KeyType &key, const KeyComparator &comparator) {
  int target_index = KeyIndex(key, comparator);
  //  没找到
  if (target_index >= GetSize() || comparator(key, KeyAt(target_index)) != 0) {
    return GetSize();
  }
  //  quick deletion
  memmove(array_ + target_index, array_ + target_index + 1,
          static_cast<size_t>((GetSize() - target_index - 1) * sizeof(MappingType)));
  IncreaseSize(-1);
  return GetSize();
}

/*****************************************************************************
 * MERGE
 *****************************************************************************/
/*
 * Remove all of key & value pairs from this page to "recipient" page. Don't forget
 * to update the next_page id in the sibling page
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::MoveAllTo(BPlusTreeLeafPage *recipient) {
  recipient->CopyNFrom(array_, GetSize());
  SetSize(0);
  recipient->SetNextPageId(next_page_id_);
}

/*****************************************************************************
 * REDISTRIBUTE
 *****************************************************************************/
/*
 * Remove the first key & value pair from this page to "recipient" page.
 *
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::MoveFirstToEndOf(BPlusTreeLeafPage *recipient) {
  recipient->CopyNFrom(array_, 1);
  for (int i = 0; i < GetSize() - 1; ++i) {
    array_[i] = array_[i + 1];
  }
  IncreaseSize(-1);
}

/*
 * Copy the item into the end of my item list. (Append item to my array)
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::CopyLastFrom(const MappingType &item) {
  assert(GetSize() + 1 <= GetMaxSize());
  array_[GetSize()] = item;
  IncreaseSize(1);
}

/*
 * Remove the last key & value pair from this page to "recipient" page.
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::MoveLastToFrontOf(BPlusTreeLeafPage *recipient) {
  // recipient 往后移
  recipient->CopyFirstFrom(array_[GetSize() - 1]);
  IncreaseSize(-1);
}

/*
 * Insert item at the front of my items. Move items accordingly.
 *
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::CopyFirstFrom(const MappingType &item) {
  for (int i = GetSize(); i > 0; --i) {
    array_[i] = array_[i - 1];
  }
  array_[0] = item;
  IncreaseSize(1);
}

template class BPlusTreeLeafPage<int, int, BasicComparator<int>>;

template class BPlusTreeLeafPage<GenericKey<4>, RowId, GenericComparator<4>>;

template class BPlusTreeLeafPage<GenericKey<8>, RowId, GenericComparator<8>>;

template class BPlusTreeLeafPage<GenericKey<16>, RowId, GenericComparator<16>>;

template class BPlusTreeLeafPage<GenericKey<32>, RowId, GenericComparator<32>>;

template class BPlusTreeLeafPage<GenericKey<64>, RowId, GenericComparator<64>>;