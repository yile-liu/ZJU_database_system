#include "page/b_plus_tree_internal_page.h"
#include "index/basic_comparator.h"
#include "index/generic_key.h"

/*****************************************************************************
 * HELPER METHODS AND UTILITIES
 *****************************************************************************/
/*
 * Init method after creating a new internal page
 * Including set page type, set current size, set page id, set parent id and set
 * max page size
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::Init(page_id_t page_id, page_id_t parent_id, int max_size) {
  SetPageType(IndexPageType::INTERNAL_PAGE);
  SetLSN(INVALID_LSN);  // no use here
  SetSize(0);
  SetMaxSize(max_size);
  SetParentPageId(parent_id);
  SetPageId(page_id);
}

/*
 * Helper method to get/set the key associated with input "index"(a.k.a array offset)
 */
INDEX_TEMPLATE_ARGUMENTS
KeyType B_PLUS_TREE_INTERNAL_PAGE_TYPE::KeyAt(int index) const {
  assert(index >= 0 && index < GetSize());
  KeyType key = array_[index].first;
  return key;
}
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::SetKeyAt(int index, const KeyType &key) {
  assert(index >= 0 && index < GetSize());
  array_[index].first = key;
}

/*
 * Helper method to get the value associated with input "index"(a.k.a array offset)
 */
INDEX_TEMPLATE_ARGUMENTS
ValueType B_PLUS_TREE_INTERNAL_PAGE_TYPE::ValueAt(int index) const {
  assert(index >= 0 && index < GetSize());
  return array_[index].second;
}

/*
 * Helper method to find and return array index(or offset), so that its value
 * equals to input "value"
 */
INDEX_TEMPLATE_ARGUMENTS
int B_PLUS_TREE_INTERNAL_PAGE_TYPE::ValueIndex(const ValueType &value) const {
  // todo 以及找不到value时怎么处理？ 现在是return -1
  // value 是页号 没有顺序 不能二分查找
  for (int i = 0; i < GetSize(); ++i) {
    if (array_[i].second == value) return i;
  }
  return -1;
}

/*****************************************************************************
 * LOOKUP
 *****************************************************************************/
/*
 * Find and return the child pointer(page_id) which points to the child page
 * that contains input "key"
 * Start the search from the second key(the first key should always be invalid)
 */
INDEX_TEMPLATE_ARGUMENTS
ValueType B_PLUS_TREE_INTERNAL_PAGE_TYPE::Lookup(const KeyType &key, const KeyComparator &comparator) const {
  assert(GetSize() > 0);
  if (comparator(key, array_[1].first) < 0) {
    return array_[0].second;
  }
  if (comparator(key, array_[GetSize() - 1].first) >= 0) {
    return array_[GetSize() - 1].second;
  }

  int left = 1;
  int right = GetSize() - 1;
  int targetIndex = -1;

  while (left <= right) {
    int mid = left + (right - left) / 2;
    int compareResult = comparator(array_[mid].first, key);
    if (compareResult == 0) {
      targetIndex = mid;
      break;
    }
    if (compareResult < 0) {
      left = mid + 1;
    } else {
      right = mid - 1;
    }
  }
  if (targetIndex != -1) {
    return array_[targetIndex].second;
  }
  return array_[left - 1].second;
}

/*****************************************************************************
 * INSERTION
 *****************************************************************************/
/*
 * Populate new root page with old_value + new_key & new_value
 * When the insertion cause overflow from leaf page all the way upto the root
 * page, you should create a new root page and populate its elements.
 * NOTE: This method is only called within InsertIntoParent()(b_plus_tree.cpp)
 */
// 借鉴版
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::PopulateNewRoot(const ValueType &old_value, const KeyType &new_key,
                                                     const ValueType &new_value) {
  array_[0].second = old_value;
  array_[1].first = new_key;
  array_[1].second = new_value;
  SetSize(2);
}

/*
 * Insert new_key & new_value pair right after the pair with its value ==
 * old_value
 * @return:  new size after insertion
 */
INDEX_TEMPLATE_ARGUMENTS
int B_PLUS_TREE_INTERNAL_PAGE_TYPE::InsertNodeAfter(const ValueType &old_value, const KeyType &new_key,
                                                    const ValueType &new_value) {
  int index = ValueIndex(old_value) + 1;
  assert(index > 0);
  IncreaseSize(1);
  for (int i = GetSize() - 1; i > index; --i) {
    array_[i] = array_[i - 1];
  }
  array_[index].first = new_key;
  array_[index].second = new_value;
  return GetSize();
}

/*****************************************************************************
 * SPLIT
 *****************************************************************************/
/*
 * Remove half of key & value pairs from this page to "recipient" page
 */
// xtodo 未测试
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::MoveHalfTo(BPlusTreeInternalPage *recipient,
                                                BufferPoolManager *buffer_pool_manager) {
  assert(recipient != nullptr);
  int total = GetMaxSize() + 1;
  assert(GetSize() == total);
  // copy last half
  int copyIdx = (total) / 2;  // max:4 x,1,2,3,4 -> 2,3,4
  page_id_t recipPageId = recipient->GetPageId();
  for (int i = copyIdx; i < total; i++) {
    recipient->array_[i - copyIdx].first = array_[i].first;
    recipient->array_[i - copyIdx].second = array_[i].second;
    // update children's parent page
    auto childRawPage = buffer_pool_manager->FetchPage(array_[i].second);
    BPlusTreePage *childTreePage = reinterpret_cast<BPlusTreePage *>(childRawPage->GetData());
    childTreePage->SetParentPageId(recipPageId);
    buffer_pool_manager->UnpinPage(array_[i].second, true);
  }
  // set size,is odd, bigger is last part
  SetSize(copyIdx);
  recipient->SetSize(total - copyIdx);
}

/*
 * Copy entries into me, starting from {items} and copy {size} entries.
 * Since it is an internal page, for all entries (pages) moved, their parent page now changes to me.
 * So I need to 'adopt' them by changing their parent page id, which needs to be persisted with BufferPoolManger
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::CopyNFrom(MappingType *items, int size, BufferPoolManager *buffer_pool_manager) {
  // 复制元素
  std::copy(items, items + size, array_ + GetSize());
  //"adopt"
  BPlusTreePage *page;
  for (int i = 0; i < size; ++i) {
    page = reinterpret_cast<BPlusTreePage *>(buffer_pool_manager->FetchPage(array_[GetSize() + i].second));
    page->SetParentPageId(GetPageId());
    buffer_pool_manager->UnpinPage(page->GetPageId(), true);
  }
  // 更新size
  IncreaseSize(size);
}

/*****************************************************************************
 * REMOVE
 *****************************************************************************/
/*
 * Remove the key & value pair in internal page according to input index(a.k.a array offset)
 * NOTE: store key&value pair continuously after deletion
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::Remove(int index) {
  assert(index >= 0 && index < GetSize());
  for (int i = index; i < GetSize() - 1; ++i) {
    array_[i] = array_[i + 1];
  }
  IncreaseSize(-1);
}

/*
 * Remove the only key & value pair in internal page and return the value
 * NOTE: only call this method within AdjustRoot()(in b_plus_tree.cpp)
 */
INDEX_TEMPLATE_ARGUMENTS
ValueType B_PLUS_TREE_INTERNAL_PAGE_TYPE::RemoveAndReturnOnlyChild() {
  ASSERT(GetSize() == 1, "not only one child in this node now, cannot run RemoveAndReturnOnlyChild()!");
  ValueType val = array_[0].second;
  SetSize(0);
  return val;
}

/*****************************************************************************
 * MERGE
 *****************************************************************************/
/*
 * Remove all of key & value pairs from this page to "recipient" page.
 * The middle_key is the separation key you should get from the parent. You need
 * to make sure the middle key is added to the recipient to maintain the invariant.
 * You also need to use BufferPoolManager to persist changes to the parent page id for those
 * pages that are moved to the recipient
 */

// xtodo 未测试
// 因为array_[0]的key不可用 所以将其move到非0位置后要补一个key 就是middle_key
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::MoveAllTo(BPlusTreeInternalPage *recipient, const KeyType &middle_key,
                                               BufferPoolManager *buffer_pool_manager) {
  recipient->CopyNFrom(array_, GetSize(), buffer_pool_manager);
  recipient->array_[recipient->GetSize() - this->GetSize()].first = middle_key;
  SetSize(0);
}

/*****************************************************************************
 * REDISTRIBUTE
 *****************************************************************************/
/*
 * Remove the first key & value pair from this page to tail of "recipient" page.
 *
 * The middle_key is the separation key you should get from the parent. You need
 * to make sure the middle key is added to the recipient to maintain the invariant.
 * You also need to use BufferPoolManager to persist changes to the parent page id for those
 * pages that are moved to the recipient
 */
// middle key同MoveAllTo
// xtodo 未测试
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::MoveFirstToEndOf(BPlusTreeInternalPage *recipient, const KeyType &middle_key,
                                                      BufferPoolManager *buffer_pool_manager) {
  array_[0].first = middle_key;
  recipient->CopyLastFrom(array_[0], buffer_pool_manager);
  for (int i = 0; i < GetSize() - 1; ++i) {
    array_[i] = array_[i + 1];
  }
  IncreaseSize(-1);
}

/* Append an entry at the end.
 * Since it is an internal page, the moved entry(page)'s parent needs to be updated.
 * So I need to 'adopt' it by changing its parent page id, which needs to be persisted with BufferPoolManger
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::CopyLastFrom(const MappingType &pair, BufferPoolManager *buffer_pool_manager) {
  array_[GetSize()] = pair;
  BPlusTreePage *page = reinterpret_cast<BPlusTreePage *>(buffer_pool_manager->FetchPage(pair.second));
  page->SetParentPageId(GetPageId());
  IncreaseSize(1);
  buffer_pool_manager->UnpinPage(page->GetPageId(), true);
}

/*
 * Remove the last key & value pair from this page to head of "recipient" page.
 * You need to handle the original dummy key properly, e.g. updating recipient’s array to position the middle_key at the
 * right place.
 * You also need to use BufferPoolManager to persist changes to the parent page id for those pages that are
 * moved to the recipient
 */
// middle key同MoveAllTo
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::MoveLastToFrontOf(BPlusTreeInternalPage *recipient, const KeyType &middle_key,
                                                       BufferPoolManager *buffer_pool_manager) {
  recipient->CopyFirstFrom(array_[GetSize() - 1], buffer_pool_manager);
  recipient->array_[1].first = middle_key;
  IncreaseSize(-1);
}

/*
 * Append an entry at the beginning.
 * Since it is an internal page, the moved entry(page)'s parent needs to be updated.
 * So I need to 'adopt' it by changing its parent page id, which needs to be persisted with BufferPoolManger
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::CopyFirstFrom(const MappingType &pair, BufferPoolManager *buffer_pool_manager) {
  for (int i = GetSize(); i > 0; --i) {
    array_[i] = array_[i - 1];
  }
  array_[0] = pair;
  BPlusTreePage *page = reinterpret_cast<BPlusTreePage *>(buffer_pool_manager->FetchPage(pair.second));
  page->SetParentPageId(GetPageId());
  IncreaseSize(1);
  buffer_pool_manager->UnpinPage(page->GetPageId(), true);
}

template class BPlusTreeInternalPage<int, int, BasicComparator<int>>;

template class BPlusTreeInternalPage<GenericKey<4>, page_id_t, GenericComparator<4>>;

template class BPlusTreeInternalPage<GenericKey<8>, page_id_t, GenericComparator<8>>;

template class BPlusTreeInternalPage<GenericKey<16>, page_id_t, GenericComparator<16>>;

template class BPlusTreeInternalPage<GenericKey<32>, page_id_t, GenericComparator<32>>;

template class BPlusTreeInternalPage<GenericKey<64>, page_id_t, GenericComparator<64>>;