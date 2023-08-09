#include "buffer/lru_replacer.h"

LRUReplacer::LRUReplacer(size_t num_pages) {
  max_size_ = num_pages;
  in_size_ = 0;
}

LRUReplacer::~LRUReplacer() = default;

bool LRUReplacer::Victim(frame_id_t *frame_id) {
  if (!in_size_) {
    return false;
  }
  //  链表末尾是最不常被访问的页，通过*frame_id返回这一页
  *frame_id = lru_list_.back();
  //  通过map里面储存的迭代器在lru_list_里删除掉这一页
  lru_list_.erase(map_[*frame_id]);
  //  在map里删除这一页
  map_.erase(*frame_id);
  --in_size_;
  return true;
}

void LRUReplacer::Pin(frame_id_t frame_id) {
  //  如果能找到
  if (map_.find(frame_id) != map_.end()) {
    lru_list_.erase(map_[frame_id]);
    map_.erase(frame_id);
    --in_size_;
  }
}

void LRUReplacer::Unpin(frame_id_t frame_id) {
  //  如果该页已经在lrureplacer里面了或者in_size_已经等于max_size_，则return
  if (map_.find(frame_id) != map_.end() || in_size_ >= max_size_) return;
  lru_list_.push_front(frame_id);
  map_[frame_id] = lru_list_.begin();
  ++in_size_;
}

size_t LRUReplacer::Size() { return in_size_; }