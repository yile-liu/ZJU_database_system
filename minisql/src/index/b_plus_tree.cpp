#include "index/b_plus_tree.h"
#include <string>
#include "glog/logging.h"
#include "index/basic_comparator.h"
#include "index/generic_key.h"
#include "page/index_roots_page.h"

INDEX_TEMPLATE_ARGUMENTS
BPLUSTREE_TYPE::BPlusTree(index_id_t index_id, BufferPoolManager *buffer_pool_manager, const KeyComparator &comparator,
                          int leaf_max_size, int internal_max_size)
    : index_id_(index_id),
      buffer_pool_manager_(buffer_pool_manager),
      comparator_(comparator),
      leaf_max_size_(leaf_max_size),
      internal_max_size_(internal_max_size) {
  root_page_id_ = INVALID_PAGE_ID;
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::Destroy() {
  std::destroy_at(this);
}

/*
 * Helper function to decide whether current b+tree is empty
 */
INDEX_TEMPLATE_ARGUMENTS
bool BPLUSTREE_TYPE::IsEmpty() const {
  if (root_page_id_ == INVALID_PAGE_ID) return true;
  return false;
}

/*****************************************************************************
 * SEARCH
 *****************************************************************************/
/*
 * Return the only value that associated with input key
 * This method is used for point query
 * @return : true means key exists
 */
INDEX_TEMPLATE_ARGUMENTS
bool BPLUSTREE_TYPE::GetValue(const KeyType &key, std::vector<ValueType> &result, Transaction *transaction) {
  //  1.find page
  B_PLUS_TREE_LEAF_PAGE_TYPE *leaf_ = reinterpret_cast<B_PLUS_TREE_LEAF_PAGE_TYPE *>(FindLeafPage(key));
  if (leaf_ == nullptr) return false;
  //  2.find value
  ValueType V;
  bool find_ = leaf_->Lookup(key, V, comparator_);
  if (find_) {
    result.push_back(V);
  }
  //  unpin buffer pool
  buffer_pool_manager_->UnpinPage(leaf_->GetPageId(), false);
  return find_;
}

/*****************************************************************************
 * INSERTION
 *****************************************************************************/
/*
 * Insert constant key & value pair into b+ tree
 * if current tree is empty, start new tree, update root page id and insert
 * entry, otherwise insert into leaf page.
 * @return: since we only support unique key, if user try to insert duplicate
 * keys return false, otherwise return true.
 */
INDEX_TEMPLATE_ARGUMENTS
bool BPLUSTREE_TYPE::Insert(const KeyType &key, const ValueType &value, Transaction *transaction) {
  if (IsEmpty()) {
    StartNewTree(key, value);
    assert(Check());
    return true;
  }
  bool result_ = InsertIntoLeaf(key, value, transaction);
  assert(Check());
  return result_;
}
/*
 * Insert constant key & value pair into an empty tree
 * User needs to first ask for new page from buffer pool manager(NOTICE: throw
 * an "out of memory" exception if returned value is nullptr), then update b+
 * tree's root page id and insert entry directly into leaf page.
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::StartNewTree(const KeyType &key, const ValueType &value) {
  //  1.ask for new page from bpm
  page_id_t new_page_id_;
  Page *root_page_ = buffer_pool_manager_->NewPage(new_page_id_);
  assert(root_page_ != nullptr);

  B_PLUS_TREE_LEAF_PAGE_TYPE *root = reinterpret_cast<B_PLUS_TREE_LEAF_PAGE_TYPE *>(root_page_->GetData());

  //  2.update b+ tree's root page
  root->Init(new_page_id_, INVALID_PAGE_ID, leaf_max_size_);
  root_page_id_ = new_page_id_;
  UpdateRootPageId(1);
  //  3.insert entry directly into leaf page
  root->Insert(key, value, comparator_);
  buffer_pool_manager_->UnpinPage(new_page_id_, true);
}

/*
 * Insert constant key & value pair into leaf page
 * User needs to first find the right leaf page as insertion target, then look
 * through leaf page to see whether insert key exist or not. If exist, return
 * immediately, otherwise insert entry. Remember to deal with split if necessary.
 * @return: since we only support unique key, if user try to insert duplicate
 * keys return false, otherwise return true.
 */
INDEX_TEMPLATE_ARGUMENTS
bool BPLUSTREE_TYPE::InsertIntoLeaf(const KeyType &key, const ValueType &value, Transaction *transaction) {
  B_PLUS_TREE_LEAF_PAGE_TYPE *leaf_page_ = reinterpret_cast<B_PLUS_TREE_LEAF_PAGE_TYPE *>(FindLeafPage(key));
  ValueType value_;
  bool exist_ = leaf_page_->Lookup(key, value_, comparator_);
  if (exist_) {
    buffer_pool_manager_->UnpinPage(leaf_page_->GetPageId(), false);
    return false;
  }
  leaf_page_->Insert(key, value, comparator_);
  //  split
  if (leaf_page_->GetSize() > leaf_page_->GetMaxSize()) {
    B_PLUS_TREE_LEAF_PAGE_TYPE *new_leaf_page_ = Split(leaf_page_);
    InsertIntoParent(leaf_page_, new_leaf_page_->KeyAt(0), new_leaf_page_, transaction);
  }
  buffer_pool_manager_->UnpinPage(leaf_page_->GetPageId(), true);

  // UpdateRootPageId();

  return true;
}

/*
 * Split input page and return newly created page.
 * Using template N to represent either internal page or leaf page.
 * User needs to first ask for new page from buffer pool manager(NOTICE: throw
 * an "out of memory" exception if returned value is nullptr), then move half
 * of key & value pairs from input page to newly created page
 */
INDEX_TEMPLATE_ARGUMENTS
template <typename N>
N *BPLUSTREE_TYPE::Split(N *node) {
  //  1.ask for new page from buffer pool manager
  page_id_t new_page_id_;
  Page *const new_page_ = buffer_pool_manager_->NewPage(new_page_id_);
  ASSERT(new_page_ != nullptr, "OUT OF MEMORY");
  //  2.move half of key & value pairs from input page to newly created page
  N *new_node_ = reinterpret_cast<N *>(new_page_->GetData());
  //  todo 为了能跑临时写的
  if (node->IsLeafPage()) {
    reinterpret_cast<LeafPage *>(new_node_)->Init(new_page_id_, node->GetParentPageId(), leaf_max_size_);
  } else {
    reinterpret_cast<InternalPage *>(new_node_)->Init(new_page_id_, node->GetParentPageId(), internal_max_size_);
  }
  if (node->IsLeafPage()) {
    reinterpret_cast<LeafPage *>(node)->MoveHalfTo(reinterpret_cast<LeafPage *>(new_node_));
  } else {
    reinterpret_cast<InternalPage *>(node)->MoveHalfTo(reinterpret_cast<InternalPage *>(new_node_),
                                                       buffer_pool_manager_);
  }
  //  fetch page and new page need to unpin page(do it outside)
  return new_node_;
}

/*
 * Insert key & value pair into internal page after split
 * @param   old_node      input page from split() method
 * @param   key
 * @param   new_node      returned page from split() method
 * User needs to first find the parent page of old_node, parent node must be
 * adjusted to take info of new_node into account. Remember to deal with split
 * recursively if necessary.
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::InsertIntoParent(BPlusTreePage *old_node, const KeyType &key, BPlusTreePage *new_node,
                                      Transaction *transaction) {
  if (old_node->IsRootPage()) {
    Page *const new_page_ = buffer_pool_manager_->NewPage(root_page_id_);
    assert(new_page_ != nullptr);
    assert(new_page_->GetPinCount() == 1);
    InternalPage *new_root_ = reinterpret_cast<InternalPage *>(new_page_->GetData());
    new_root_->Init(root_page_id_, INVALID_PAGE_ID, internal_max_size_);
    new_root_->PopulateNewRoot(old_node->GetPageId(), key, new_node->GetPageId());
    old_node->SetParentPageId(root_page_id_);
    new_node->SetParentPageId(root_page_id_);
    UpdateRootPageId();
    //    fetchpage and new page need to unpin page
    buffer_pool_manager_->UnpinPage(new_node->GetPageId(), true);
    buffer_pool_manager_->UnpinPage(new_root_->GetPageId(), true);
    return;
  }
  page_id_t parent_page_id = old_node->GetParentPageId();
  auto *page_ = FetchPage(parent_page_id);
  assert(page_ != nullptr);
  InternalPage *parent_ = reinterpret_cast<InternalPage *>(page_);
  new_node->SetParentPageId(parent_page_id);
  buffer_pool_manager_->UnpinPage(new_node->GetPageId(), true);
  parent_->InsertNodeAfter(old_node->GetPageId(), key, new_node->GetPageId());
  if (parent_->GetSize() > parent_->GetMaxSize()) {
    //    begin /* Split Parent*/
    InternalPage *new_leaf_page_ = Split(parent_);
    InsertIntoParent(parent_, new_leaf_page_->KeyAt(0), new_leaf_page_, transaction);
  }
  buffer_pool_manager_->UnpinPage(parent_page_id, true);
}

/*****************************************************************************
 * REMOVE
 *****************************************************************************/
/*
 * Delete key & value pair associated with input key
 * If current tree is empty, return immediately.
 * If not, User needs to first find the right leaf page as deletion target, then
 * delete entry from leaf page. Remember to deal with redistribute or merge if
 * necessary.
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::Remove(const KeyType &key, Transaction *transaction) {
  if (IsEmpty()) return;
  B_PLUS_TREE_LEAF_PAGE_TYPE *target_ = reinterpret_cast<B_PLUS_TREE_LEAF_PAGE_TYPE *>(FindLeafPage(key));
  if (target_->KeyIndex(key,comparator_)==0){
    if (!target_->IsRootPage()){
      InternalPage* parent_page= reinterpret_cast<InternalPage *>(FetchPage(target_->GetParentPageId()));
      int page_index=parent_page->ValueIndex(target_->GetPageId());
      parent_page->SetKeyAt(page_index,target_->KeyAt(1));
      buffer_pool_manager_->UnpinPage(target_->GetParentPageId(), true);
    }
  }
  int after_del_size_ = target_->RemoveAndDeleteRecord(key, comparator_);
  if (after_del_size_ < target_->GetMinSize()) {
    CoalesceOrRedistribute(target_, transaction);
  }
  buffer_pool_manager_->UnpinPage(target_->GetPageId(), true);
  assert(Check());
}

/*
 * User needs to first find the sibling of input page. If sibling's size + input
 * page's size > page's max size, then redistribute. Otherwise, merge.
 * Using template N to represent either internal page or leaf page.
 * @return: true means target leaf page should be deleted, false means no
 * deletion happens
 */
INDEX_TEMPLATE_ARGUMENTS
template <typename N>
bool BPLUSTREE_TYPE::CoalesceOrRedistribute(N *node, Transaction *transaction) {
  //  if N is the root and N has only one remaining child
  if (node->IsRootPage()) {
    return AdjustRoot(node);
  }
  N *node2;
  bool n_prev_N2 = FindSibling(node, node2);
  InternalPage *parent = reinterpret_cast<InternalPage *>(FetchPage(node->GetParentPageId()));
  InternalPage *parent_page = reinterpret_cast<InternalPage *>(parent);
  //  if entries in N and N2 can fit in a single node
  if (node->GetSize() + node2->GetSize() <= node->GetMaxSize()) {
    if (n_prev_N2) {
      std::swap(node, node2);
      //    assumption node is after node2;
    }
    int remove_index_ = parent->ValueIndex(node->GetPageId());
    Coalesce(&node2, &node, &parent_page, remove_index_, transaction);  // unpin node,node2
    buffer_pool_manager_->UnpinPage(parent_page->GetPageId(), true);
    return true;
  }
  //  redistribution:borrow an entry from N2
  int node_in_parent_index = parent_page->ValueIndex(node->GetPageId());
  Redistribute(node2, node, node_in_parent_index);  // unpin node,node2
  buffer_pool_manager_->UnpinPage(parent_page->GetPageId(), false);
  return false;
}

INDEX_TEMPLATE_ARGUMENTS
template <typename N>
bool BPLUSTREE_TYPE::FindSibling(N *node, N *&sibling) {
  auto page = FetchPage(node->GetParentPageId());
  BPlusTreeInternalPage<KeyType, page_id_t, KeyComparator> *parent =
      reinterpret_cast<BPlusTreeInternalPage<KeyType, page_id_t, KeyComparator> *>(page);
  int index = parent->ValueIndex(node->GetPageId());
  int siblingIndex = index - 1;
  if (index == 0) {  // no prev sibling
    siblingIndex = index + 1;
  }
  buffer_pool_manager_->UnpinPage(page->GetPageId(), false);
  sibling = reinterpret_cast<N *>(FetchPage(parent->ValueAt(siblingIndex)));
  buffer_pool_manager_->UnpinPage(parent->GetPageId(), false);
  return index == 0;  // index == 0 means sibling is post node
}

/*
 * Move all the key & value pairs from one page to its sibling page, and notify
 * buffer pool manager to delete this page. Parent page must be adjusted to
 * take info of deletion into account. Remember to deal with coalesce or
 * redistribute recursively if necessary.
 * Using template N to represent either internal page or leaf page.
 * @param   neighbor_node      sibling page of input "node"
 * @param   node               input from method coalesceOrRedistribute()
 * @param   parent             parent page of input "node"
 * @return  true means parent node should be deleted, false means no deletion happened
 */
INDEX_TEMPLATE_ARGUMENTS
template <typename N>
bool BPLUSTREE_TYPE::Coalesce(N **neighbor_node, N **node,
                              BPlusTreeInternalPage<KeyType, page_id_t, KeyComparator> **parent, int index,
                              Transaction *transaction) {
  //  asuumption neighbor_node is before node
  int key_index_ = index;
  KeyType middle_key_ = (*parent)->KeyAt(key_index_);
  if ((*node)->IsLeafPage()) {
    LeafPage *leaf_node_ = reinterpret_cast<LeafPage *>((*node));
    LeafPage *prev_leaf_node_ = reinterpret_cast<LeafPage *>((*neighbor_node));
    leaf_node_->MoveAllTo(reinterpret_cast<LeafPage *>(prev_leaf_node_));
  } else {
    InternalPage *internal_node_ = reinterpret_cast<InternalPage *>((*node));
    InternalPage *prev_internal_node = reinterpret_cast<InternalPage *>((*neighbor_node));
    internal_node_->MoveAllTo(prev_internal_node, middle_key_, buffer_pool_manager_);
  }
  page_id_t pid = (*node)->GetPageId();
  buffer_pool_manager_->UnpinPage(pid, true);
  buffer_pool_manager_->DeletePage(pid);
  buffer_pool_manager_->UnpinPage((*neighbor_node)->GetPageId(), true);
  (*parent)->Remove(key_index_);
  if ((*parent)->GetSize() < (*parent)->GetMinSize()) {
    return CoalesceOrRedistribute(*parent, transaction);
  }
  return false;
}

/*
 * Redistribute key & value pairs from one page to its sibling page. If index ==
 * 0, move sibling page's first key & value pair into end of input "node",
 * otherwise move sibling page's last key & value pair into head of input
 * "node".
 * Using template N to represent either internal page or leaf page.
 * @param   neighbor_node      sibling page of input "node"
 * @param   node               input from method coalesceOrRedistribute()
 */
INDEX_TEMPLATE_ARGUMENTS
template <typename N>
void BPLUSTREE_TYPE::Redistribute(N *neighbor_node, N *node, int index) {
  // if IsLeaf
  // nei_n = re_cast<>()
  //  cout<<"Redistribute:"<<" neighbornode: "<<reinterpret_cast<LeafPage *>(neighbor_node)->KeyAt(0)<<"
  //  node:"<<node->KeyAt(0)<<" index: "<<index<<endl;
  if (index == 0) {
    if (node->IsLeafPage()) {
      auto pre_middle_key_leaf_page = FindLeafPage(
          reinterpret_cast<LeafPage *>(node)->KeyAt(reinterpret_cast<LeafPage *>(node)->GetSize() - 1), false);
      auto middle_key_leaf_page = FetchPage(reinterpret_cast<LeafPage *>(pre_middle_key_leaf_page)->GetNextPageId());
      auto aft_middle_key_leaf_page = FetchPage(reinterpret_cast<LeafPage *>(middle_key_leaf_page)->GetNextPageId());
      InternalPage *parent = reinterpret_cast<InternalPage *>(FetchPage(neighbor_node->GetParentPageId()));
      int neighbot_node_index = parent->ValueIndex(neighbor_node->GetPageId());
      parent->SetKeyAt(neighbot_node_index, reinterpret_cast<LeafPage *>(middle_key_leaf_page)->KeyAt(1));
      reinterpret_cast<LeafPage *>(neighbor_node)->MoveFirstToEndOf(reinterpret_cast<LeafPage *>(node));
      buffer_pool_manager_->UnpinPage(parent->GetPageId(), true);
      buffer_pool_manager_->UnpinPage(middle_key_leaf_page->GetPageId(), false);
      buffer_pool_manager_->UnpinPage(aft_middle_key_leaf_page->GetPageId(), false);
    } else {
      auto pre_middle_key_leaf_page = FindLeafPage(
          reinterpret_cast<InternalPage *>(node)->KeyAt(reinterpret_cast<InternalPage *>(node)->GetSize() - 1), false);
      auto middle_key_leaf_page = FetchPage(reinterpret_cast<LeafPage *>(pre_middle_key_leaf_page)->GetNextPageId());
      auto aft_middle_key_leaf_page = FetchPage(reinterpret_cast<LeafPage *>(middle_key_leaf_page)->GetNextPageId());
      KeyType middle_key = reinterpret_cast<LeafPage *>(middle_key_leaf_page)->KeyAt(0);
      InternalPage *parent = reinterpret_cast<InternalPage *>(FetchPage(node->GetParentPageId()));
      int neighbot_node_index = parent->ValueIndex(neighbor_node->GetPageId());
      parent->SetKeyAt(neighbot_node_index, reinterpret_cast<LeafPage *>(aft_middle_key_leaf_page)->KeyAt(0));
      reinterpret_cast<InternalPage *>(neighbor_node)
          ->MoveFirstToEndOf(reinterpret_cast<InternalPage *>(node), middle_key, buffer_pool_manager_);
      buffer_pool_manager_->UnpinPage(parent->GetPageId(), true);
      buffer_pool_manager_->UnpinPage(middle_key_leaf_page->GetPageId(), false);
      buffer_pool_manager_->UnpinPage(aft_middle_key_leaf_page->GetPageId(), false);
    }
  } else {
    if (node->IsLeafPage()) {
      auto pre_middle_key_leaf_page =
          FindLeafPage(reinterpret_cast<LeafPage *>(neighbor_node)
                           ->KeyAt(reinterpret_cast<LeafPage *>(neighbor_node)->GetSize() - 1),
                       false);
      auto middle_key_leaf_page = FetchPage(reinterpret_cast<LeafPage *>(pre_middle_key_leaf_page)->GetNextPageId());
      auto aft_middle_key_leaf_page = FetchPage(reinterpret_cast<LeafPage *>(middle_key_leaf_page)->GetNextPageId());
      InternalPage *parent = reinterpret_cast<InternalPage *>(FetchPage(neighbor_node->GetParentPageId()));
      int node_index = parent->ValueIndex(node->GetPageId());
      parent->SetKeyAt(node_index, reinterpret_cast<LeafPage *>(pre_middle_key_leaf_page)
                                       ->KeyAt(reinterpret_cast<LeafPage *>(pre_middle_key_leaf_page)->GetSize() - 1));
      reinterpret_cast<LeafPage *>(neighbor_node)->MoveLastToFrontOf(reinterpret_cast<LeafPage *>(node));
      buffer_pool_manager_->UnpinPage(parent->GetPageId(), true);
      buffer_pool_manager_->UnpinPage(middle_key_leaf_page->GetPageId(), false);
      buffer_pool_manager_->UnpinPage(aft_middle_key_leaf_page->GetPageId(), false);
    } else {
      auto pre_middle_key_leaf_page =
          FindLeafPage(reinterpret_cast<InternalPage *>(neighbor_node)
                           ->KeyAt(reinterpret_cast<InternalPage *>(neighbor_node)->GetSize() - 1),
                       false);
      auto middle_key_leaf_page = FetchPage(reinterpret_cast<LeafPage *>(pre_middle_key_leaf_page)->GetNextPageId());
      auto aft_middle_key_leaf_page = FetchPage(reinterpret_cast<LeafPage *>(middle_key_leaf_page)->GetNextPageId());
      KeyType middle_key = reinterpret_cast<LeafPage *>(middle_key_leaf_page)->KeyAt(0);
      InternalPage *parent = reinterpret_cast<InternalPage *>(FetchPage(node->GetParentPageId()));
      int node_index = parent->ValueIndex(node->GetPageId());
      parent->SetKeyAt(node_index, reinterpret_cast<LeafPage *>(pre_middle_key_leaf_page)->KeyAt(0));
      reinterpret_cast<InternalPage *>(neighbor_node)
          ->MoveLastToFrontOf(reinterpret_cast<InternalPage *>(node), middle_key, buffer_pool_manager_);
      buffer_pool_manager_->UnpinPage(parent->GetPageId(), true);
      buffer_pool_manager_->UnpinPage(middle_key_leaf_page->GetPageId(), false);
      buffer_pool_manager_->UnpinPage(aft_middle_key_leaf_page->GetPageId(), false);
    }
  }
  buffer_pool_manager_->UnpinPage(node->GetPageId(), true);
  buffer_pool_manager_->UnpinPage(neighbor_node->GetPageId(), true);
}

/*
 * Update root page if necessary
 * NOTE: size of root page can be less than min size and this method is only
 * called within coalesceOrRedistribute() method
 * case 1: when you delete the last element in root page, but root page still
 * has one last child
 * case 2: when you delete the last element in whole b+ tree
 * @return : true means root page should be deleted, false means no deletion
 * happened
 */
INDEX_TEMPLATE_ARGUMENTS
bool BPLUSTREE_TYPE::AdjustRoot(BPlusTreePage *old_root_node) {
  if (old_root_node->IsLeafPage()) {  // case 2
    assert(old_root_node->GetSize() == 0);
    assert(old_root_node->GetParentPageId() == INVALID_PAGE_ID);
    buffer_pool_manager_->UnpinPage(old_root_node->GetPageId(), false);
    buffer_pool_manager_->DeletePage(old_root_node->GetPageId());
    root_page_id_ = INVALID_PAGE_ID;
    UpdateRootPageId();
    return true;
  }
  if (old_root_node->GetSize() == 1) {  // case 1
    InternalPage *root = reinterpret_cast<InternalPage *>(old_root_node);
    const page_id_t new_root_id = root->RemoveAndReturnOnlyChild();
    root_page_id_ = new_root_id;
    UpdateRootPageId();
    //    set the new root's parent id invalid
    auto *page = FetchPage(root_page_id_);
    assert(page != nullptr);
    InternalPage *new_root = reinterpret_cast<InternalPage *>(page);
    new_root->SetParentPageId(INVALID_PAGE_ID);
    buffer_pool_manager_->UnpinPage(root_page_id_, true);
    buffer_pool_manager_->UnpinPage(old_root_node->GetPageId(), false);
    buffer_pool_manager_->DeletePage(old_root_node->GetPageId());
  }
  return false;
}

/*****************************************************************************
 * INDEX ITERATOR
 *****************************************************************************/
/*
 * Input parameter is void, find the left most leaf page first, then construct
 * index iterator
 * @return : index iterator
 */
INDEX_TEMPLATE_ARGUMENTS
INDEXITERATOR_TYPE BPLUSTREE_TYPE::Begin() {
  KeyType useless;
  auto start_leaf = FindLeafPage(useless, true);
  return INDEXITERATOR_TYPE(reinterpret_cast<LeafPage *>(start_leaf), buffer_pool_manager_, 0);
}

/*
 * Input parameter is low key, find the leaf page that contains the input key
 * first, then construct index iterator
 * @return : index iterator
 */
INDEX_TEMPLATE_ARGUMENTS
INDEXITERATOR_TYPE BPLUSTREE_TYPE::Begin(const KeyType &key) {
  auto start_leaf = FindLeafPage(key);
  if (start_leaf == nullptr) {
    return INDEXITERATOR_TYPE(reinterpret_cast<LeafPage *>(start_leaf), buffer_pool_manager_, 0);
  }
  int idx = reinterpret_cast<LeafPage *>(start_leaf)->KeyIndex(key, comparator_);
  return INDEXITERATOR_TYPE(reinterpret_cast<LeafPage *>(start_leaf), buffer_pool_manager_, idx);
}

/*
 * Input parameter is void, construct an index iterator representing the end
 * of the key/value pair in the leaf node
 * @return : index iterator
 */
INDEX_TEMPLATE_ARGUMENTS
INDEXITERATOR_TYPE BPLUSTREE_TYPE::End() {
  auto root_page = FetchPage(root_page_id_);
  if (root_page->IsLeafPage()) {
    LeafPage *root_;
    root_ = reinterpret_cast<LeafPage *>(root_page);
    KeyType end_key = root_->KeyAt(root_->GetSize() - 1);
    auto end_leaf = FindLeafPage(end_key);
    buffer_pool_manager_->UnpinPage(root_page_id_, false);
    return INDEXITERATOR_TYPE(reinterpret_cast<LeafPage *>(end_leaf), buffer_pool_manager_,
                              reinterpret_cast<LeafPage *>(end_leaf)->GetSize() - 1);

  } else {
    InternalPage *root_;
    root_ = reinterpret_cast<InternalPage *>(root_page);
    KeyType end_key = root_->KeyAt(root_->GetSize() - 1);
    auto end_leaf = FindLeafPage(end_key);
    buffer_pool_manager_->UnpinPage(root_page_id_, false);
    return INDEXITERATOR_TYPE(reinterpret_cast<LeafPage *>(end_leaf), buffer_pool_manager_,
                              reinterpret_cast<LeafPage *>(end_leaf)->GetSize() - 1);
  }
}

/*****************************************************************************
 * UTILITIES AND DEBUG
 *****************************************************************************/
/*
 * Find leaf page containing particular key, if leftMost flag == true, find
 * the left most leaf page
 * Note: the leaf page is pinned, you need to unpin it after use.
 */
INDEX_TEMPLATE_ARGUMENTS
Page *BPLUSTREE_TYPE::FindLeafPage(const KeyType &key, bool leftMost) {
  if (IsEmpty()) return nullptr;
  //, you need to first fetch the page from buffer pool using its unique page_id, then reinterpret cast to either
  // a leaf or an internal page, and unpin the page after any writing or reading operations.
  auto pointer = FetchPage(root_page_id_);
  page_id_t next_page_id, curr_page_id;
  for (curr_page_id = root_page_id_; !pointer->IsLeafPage();
       curr_page_id = next_page_id, pointer = FetchPage(curr_page_id)) {
    InternalPage *internalPage = static_cast<InternalPage *>(pointer);
    if (leftMost) {
      next_page_id = internalPage->ValueAt(0);
    } else {
      next_page_id = internalPage->Lookup(key, comparator_);
    }
    buffer_pool_manager_->UnpinPage(curr_page_id, false);
  }
  buffer_pool_manager_->UnpinPage(curr_page_id, false);
  return reinterpret_cast<Page *>(pointer);
}

INDEX_TEMPLATE_ARGUMENTS
BPlusTreePage *BPLUSTREE_TYPE::FetchPage(page_id_t page_id) {
  auto page = buffer_pool_manager_->FetchPage(page_id);
  return reinterpret_cast<BPlusTreePage *>(page->GetData());
}
/*
 * Update/Insert root page id in header page(where page_id = 1, index_root_page is
 * defined under include/page/index_root_page.h)
 * Call this method everytime root page id is changed.
 * @parameter: insert_record      default value is false. When set to true,
 * insert a record <index_name, root_page_id> into header page instead of
 * updating it.
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::UpdateRootPageId(int insert_record) {
  auto root_page = reinterpret_cast<IndexRootsPage *>(buffer_pool_manager_->FetchPage(1));
  if (insert_record) {
    root_page->Insert(index_id_, root_page_id_);
  } else {
    root_page->Update(index_id_, root_page_id_);
  }
  buffer_pool_manager_->UnpinPage(1, true);
}

/**
 * This method is used for debug only, You don't need to modify
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::ToGraph(BPlusTreePage *page, BufferPoolManager *bpm, std::ofstream &out) const {
  std::string leaf_prefix("LEAF_");
  std::string internal_prefix("INT_");
  if (page->IsLeafPage()) {
    auto *leaf = reinterpret_cast<LeafPage *>(page);
    // Print node name
    out << leaf_prefix << leaf->GetPageId();
    // Print node properties
    out << "[shape=plain color=green ";
    // Print data of the node
    out << "label=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\">\n";
    // Print data
    out << "<TR><TD COLSPAN=\"" << leaf->GetSize() << "\">P=" << leaf->GetPageId()
        << ",Parent=" << leaf->GetParentPageId() << "</TD></TR>\n";
    out << "<TR><TD COLSPAN=\"" << leaf->GetSize() << "\">"
        << "max_size=" << leaf->GetMaxSize() << ",min_size=" << leaf->GetMinSize() << ",size=" << leaf->GetSize()
        << "</TD></TR>\n";
    out << "<TR>";
    for (int i = 0; i < leaf->GetSize(); i++) {
      out << "<TD>" << leaf->KeyAt(i) << "</TD>\n";
    }
    out << "</TR>";
    // Print table end
    out << "</TABLE>>];\n";
    // Print Leaf node link if there is a next page
    if (leaf->GetNextPageId() != INVALID_PAGE_ID) {
      out << leaf_prefix << leaf->GetPageId() << " -> " << leaf_prefix << leaf->GetNextPageId() << ";\n";
      out << "{rank=same " << leaf_prefix << leaf->GetPageId() << " " << leaf_prefix << leaf->GetNextPageId() << "};\n";
    }

    // Print parent links if there is a parent
    if (leaf->GetParentPageId() != INVALID_PAGE_ID) {
      out << internal_prefix << leaf->GetParentPageId() << ":p" << leaf->GetPageId() << " -> " << leaf_prefix
          << leaf->GetPageId() << ";\n";
    }
  } else {
    auto *inner = reinterpret_cast<InternalPage *>(page);
    // Print node name
    out << internal_prefix << inner->GetPageId();
    // Print node properties
    out << "[shape=plain color=pink ";  // why not?
    // Print data of the node
    out << "label=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\">\n";
    // Print data
    out << "<TR><TD COLSPAN=\"" << inner->GetSize() << "\">P=" << inner->GetPageId()
        << ",Parent=" << inner->GetParentPageId() << "</TD></TR>\n";
    out << "<TR><TD COLSPAN=\"" << inner->GetSize() << "\">"
        << "max_size=" << inner->GetMaxSize() << ",min_size=" << inner->GetMinSize() << ",size=" << inner->GetSize()
        << "</TD></TR>\n";
    out << "<TR>";
    for (int i = 0; i < inner->GetSize(); i++) {
      out << "<TD PORT=\"p" << inner->ValueAt(i) << "\">";
      if (i > 0) {
        out << inner->KeyAt(i);
      } else {
        out << " ";
      }
      out << "</TD>\n";
    }
    out << "</TR>";
    // Print table end
    out << "</TABLE>>];\n";
    // Print Parent link
    if (inner->GetParentPageId() != INVALID_PAGE_ID) {
      out << internal_prefix << inner->GetParentPageId() << ":p" << inner->GetPageId() << " -> " << internal_prefix
          << inner->GetPageId() << ";\n";
    }
    // Print leaves
    for (int i = 0; i < inner->GetSize(); i++) {
      auto child_page = reinterpret_cast<BPlusTreePage *>(bpm->FetchPage(inner->ValueAt(i))->GetData());
      ToGraph(child_page, bpm, out);
      if (i > 0) {
        auto sibling_page = reinterpret_cast<BPlusTreePage *>(bpm->FetchPage(inner->ValueAt(i - 1))->GetData());
        if (!sibling_page->IsLeafPage() && !child_page->IsLeafPage()) {
          out << "{rank=same " << internal_prefix << sibling_page->GetPageId() << " " << internal_prefix
              << child_page->GetPageId() << "};\n";
        }
        bpm->UnpinPage(sibling_page->GetPageId(), false);
      }
    }
  }
  bpm->UnpinPage(page->GetPageId(), false);
}

/**
 * This function is for debug only, you don't need to modify
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::ToString(BPlusTreePage *page, BufferPoolManager *bpm) const {
  if (page->IsLeafPage()) {
    auto *leaf = reinterpret_cast<LeafPage *>(page);
    std::cout << "Leaf Page: " << leaf->GetPageId() << " parent: " << leaf->GetParentPageId()
              << " next: " << leaf->GetNextPageId() << std::endl;
    for (int i = 0; i < leaf->GetSize(); i++) {
      std::cout << leaf->KeyAt(i) << ",";
    }
    std::cout << std::endl;
    std::cout << std::endl;
  } else {
    auto *internal = reinterpret_cast<InternalPage *>(page);
    std::cout << "Internal Page: " << internal->GetPageId() << " parent: " << internal->GetParentPageId() << std::endl;
    for (int i = 0; i < internal->GetSize(); i++) {
      std::cout << internal->KeyAt(i) << ": " << internal->ValueAt(i) << ",";
    }
    std::cout << std::endl;
    std::cout << std::endl;
    for (int i = 0; i < internal->GetSize(); i++) {
      ToString(reinterpret_cast<BPlusTreePage *>(bpm->FetchPage(internal->ValueAt(i))->GetData()), bpm);
      bpm->UnpinPage(internal->ValueAt(i), false);
    }
  }
}

INDEX_TEMPLATE_ARGUMENTS
bool BPLUSTREE_TYPE::Check() {
  bool all_unpinned = buffer_pool_manager_->CheckAllUnpinned();
  if (!all_unpinned) {
    LOG(ERROR) << "problem in page unpin" << endl;
  }
  return all_unpinned;
}

template class BPlusTree<int, int, BasicComparator<int>>;

template class BPlusTree<GenericKey<4>, RowId, GenericComparator<4>>;

template class BPlusTree<GenericKey<8>, RowId, GenericComparator<8>>;

template class BPlusTree<GenericKey<16>, RowId, GenericComparator<16>>;

template class BPlusTree<GenericKey<32>, RowId, GenericComparator<32>>;

template class BPlusTree<GenericKey<64>, RowId, GenericComparator<64>>;
