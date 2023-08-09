#include "buffer/buffer_pool_manager.h"
#include "glog/logging.h"
#include "page/bitmap_page.h"

//#define OUTPUT_PAGE_ID_FOR_DEBUG

BufferPoolManager::BufferPoolManager(size_t pool_size, DiskManager *disk_manager)
    : pool_size_(pool_size), disk_manager_(disk_manager) {
  pages_ = new Page[pool_size_];
  replacer_ = new LRUReplacer(pool_size_);
  for (size_t i = 0; i < pool_size_; i++) {
    free_list_.emplace_back(i);
  }
}

BufferPoolManager::~BufferPoolManager() {
  for (auto page : page_table_) {
    FlushPage(page.first);
  }
  delete[] pages_;
  delete replacer_;
}

Page *BufferPoolManager::FetchPage(page_id_t page_id) {
#ifdef OUTPUT_PAGE_ID_FOR_DEBUG
  cout << "BufferPoolManager::FetchPage " << page_id << endl;
#endif

  // 1.     Search the page table for the requested page (P).
  // 1.1    If P exists, pin it and return it immediately.
  if (page_table_.find(page_id) != page_table_.end()) {
    frame_id_t frame_id;
    frame_id = page_table_[page_id];
    replacer_->Pin(frame_id);
    ++pages_[frame_id].pin_count_;
    return &pages_[frame_id];
  }
  // 1.2    If P does not exist, find a replacement page (R) from either the free list or the replacer.
  //        Note that pages are always found from the free list first.
  frame_id_t frame_id;
  if (!free_list_.empty()) {
    frame_id = free_list_.back();
    free_list_.pop_back();
  } else {
    replacer_->Victim(&frame_id);
  }
  // 2.     If R is dirty, write it back to the disk.
  if (pages_[frame_id].IsDirty()) {
    FlushPage(pages_[frame_id].page_id_);
    pages_[frame_id].is_dirty_ = false;
  }
  // 3.     Delete R from the page table and insert P.
  page_table_.erase(pages_[frame_id].page_id_);
  page_table_[page_id] = frame_id;
  // 4.     Update P's metadata, read in the page content from disk, and then return a pointer to P.
  pages_[frame_id].page_id_ = page_id;
  pages_[frame_id].pin_count_ += 1;
  pages_[frame_id].ResetMemory();
  disk_manager_->ReadPage(page_id, pages_[frame_id].GetData());
  return &pages_[frame_id];
}

Page *BufferPoolManager::NewPage(page_id_t &page_id) {
  // 0.   Make sure you call AllocatePage!
  frame_id_t frame_id;
  // 1.   If all the pages in the buffer pool are pinned, return nullptr.
  if (free_list_.empty() && !replacer_->Victim(&frame_id)) {
    return nullptr;
  }
  // 2.   Pick a victim page P from either the free list or the replacer. Always pick from the free list first.
  else {
    page_id = AllocatePage();
    if (!free_list_.empty()) {
      frame_id = free_list_.back();
      free_list_.pop_back();
    } else {
      replacer_->Victim(&frame_id);
    }
  }
  // 3.   Update P's metadata, zero out memory and add P to the page table.
  page_table_[page_id] = frame_id;
  pages_[frame_id].ResetMemory();
  // 4.   Set the page ID output parameter. Return a pointer to P.
  pages_[frame_id].page_id_ = page_id;
  pages_[frame_id].pin_count_ = 1;
  pages_[frame_id].is_dirty_ = true;

#ifdef OUTPUT_PAGE_ID_FOR_DEBUG
  cout << "BufferPoolManager::NewPage " << page_id << endl;
#endif

  return &pages_[frame_id];
}

bool BufferPoolManager::DeletePage(page_id_t page_id) {
#ifdef OUTPUT_PAGE_ID_FOR_DEBUG
  cout << "BufferPoolManager::Delete " << page_id << endl;
#endif

  // 0.   Make sure you call DeallocatePage!
  DeallocatePage(page_id);
  // 1.   Search the page table for the requested page (P).
  // 1.   If P does not exist, return true.
  if (page_table_.find(page_id) == page_table_.end()) {
    return true;
  }
  // 2.   If P exists, but has a non-zero pin-count, return false. Someone is using the page.
  frame_id_t frame_id;
  frame_id = page_table_[page_id];
  if (pages_[frame_id].pin_count_ != 0) {
    return false;
  }
  // 3.   Otherwise, P can be deleted. Remove P from the page table, reset its metadata and return it to the free list.
  pages_[frame_id].page_id_ = INVALID_PAGE_ID;
  pages_[frame_id].ResetMemory();
  page_table_.erase(page_id);
  free_list_.push_back(frame_id);
  return true;
}

bool BufferPoolManager::UnpinPage(page_id_t page_id, bool is_dirty) {
#ifdef OUTPUT_PAGE_ID_FOR_DEBUG
  cout << "BufferPoolManager::UnpinPage " << page_id << endl;
#endif

  //  找不到page，return false
  if (page_table_.find(page_id) == page_table_.end()) {
    return false;
  }
  //  能找到，将其Unpin
  frame_id_t frame_id;
  frame_id = page_table_[page_id];
  //  异常情况
  if (pages_[frame_id].pin_count_ < 0) {
    return false;
  }
  //  正被调用，减少一个线程
  if (pages_[frame_id].pin_count_ > 0) {
    --pages_[frame_id].pin_count_;
  }
  //  未被调用，加入replacer_
  if (pages_[frame_id].pin_count_ == 0) {
    replacer_->Unpin(frame_id);
  }
  //  设置is_dirty_
  if (is_dirty) {
    pages_[frame_id].is_dirty_ = true;
    FlushPage(page_id);
  }
  return true;
}

bool BufferPoolManager::FlushPage(page_id_t page_id) {
  //  找不到page，return false
  if (page_table_.find(page_id) == page_table_.end()) {
    return false;
  }
  //  找得到，将其写入磁盘
  frame_id_t frame_id;
  frame_id = page_table_[page_id];
  disk_manager_->WritePage(page_id, pages_[frame_id].GetData());
  pages_[frame_id].is_dirty_ = false;

  return true;
}

page_id_t BufferPoolManager::AllocatePage() {
  int next_page_id = disk_manager_->AllocatePage();
  return next_page_id;
}

void BufferPoolManager::DeallocatePage(page_id_t page_id) { disk_manager_->DeAllocatePage(page_id); }

bool BufferPoolManager::IsPageFree(page_id_t page_id) { return disk_manager_->IsPageFree(page_id); }

// Only used for debug
bool BufferPoolManager::CheckAllUnpinned() {
  bool res = true;
  for (size_t i = 0; i < pool_size_; i++) {
    if (pages_[i].pin_count_ != 0) {
      res = false;
      LOG(ERROR) << "page " << pages_[i].page_id_ << " pin count:" << pages_[i].pin_count_ << endl;
    }
  }
  return res;
}