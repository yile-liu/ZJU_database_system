#include "page/bitmap_page.h"

// 工具数组
// byte_bit[i] 第i个bit为1其余bit为0的unsigned char， 尽量不要手写magic number
constexpr unsigned char byte_bit[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
// byte_bit_not[i] 第i个bit为0其余bit为1的unsigned char
constexpr unsigned char byte_bit_not[8] = {0x7F, 0xBF, 0xDF, 0xEF, 0xF7, 0xFB, 0xFD, 0xFE};

template <size_t PageSize>
bool BitmapPage<PageSize>::AllocatePage(uint32_t &page_offset) {
  for (unsigned int i = 0; i < MAX_CHARS; ++i) {
    if (bytes[i] ^ 0xFF) {  // 如果bytes[i]不全为1，即存在未分配的块，位运算比等号快
      for (int j = 0; j < 8; ++j) {
        if (!(bytes[i] & byte_bit[j])) {  // 第j个bit是0
          bytes[i] |= byte_bit[j];        // 将这个bit设为1
          page_offset = i * 8 + j;
          return true;
        }
      }
    }
  }

  return false;
}

template <size_t PageSize>
bool BitmapPage<PageSize>::DeAllocatePage(uint32_t page_offset) {
  if (page_offset < MAX_CHARS * 8) {                            // 没有超出界限
    if (bytes[page_offset / 8] & byte_bit[page_offset % 8]) {   // 对应的bit是1
      bytes[page_offset / 8] &= byte_bit_not[page_offset % 8];  // 将bit设为0
      return true;
    }
  }
  return false;
}

template <size_t PageSize>
bool BitmapPage<PageSize>::IsPageFree(uint32_t page_offset) const {
  if (page_offset < MAX_CHARS * 8) {
    if (!(bytes[page_offset / 8] & byte_bit[page_offset % 8])) {
      return true;
    }
  }
  return false;
}

template <size_t PageSize>
bool BitmapPage<PageSize>::IsPageFreeLow(uint32_t byte_index, uint8_t bit_index) const {
  return false;
}

template class BitmapPage<64>;

template class BitmapPage<128>;

template class BitmapPage<256>;

template class BitmapPage<512>;

template class BitmapPage<1024>;

template class BitmapPage<2048>;

template class BitmapPage<4096>;