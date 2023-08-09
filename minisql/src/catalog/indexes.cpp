#include "catalog/indexes.h"

IndexMetadata *IndexMetadata::Create(const index_id_t index_id, const string &index_name, const table_id_t table_id,
                                     const vector<uint32_t> &key_map, MemHeap *heap) {
  void *buf = heap->Allocate(sizeof(IndexMetadata));
  return new (buf) IndexMetadata(index_id, index_name, table_id, key_map);
}

uint32_t IndexMetadata::SerializeTo(char *buf) const {
  uint32_t ofs = 0, move;
  MACH_WRITE_UINT32(buf, INDEX_METADATA_MAGIC_NUM);
  move = sizeof(uint32_t);
  ofs += move;
  buf += move;
  MACH_WRITE_UINT32(buf, index_id_);
  ofs += move;
  buf += move;
  MACH_WRITE_UINT32(buf, index_name_.length());
  move = sizeof(uint32_t);
  ofs += move;
  buf += move;
  MACH_WRITE_STRING(buf, index_name_);
  move = MACH_STR_SERIALIZED_SIZE(index_name_);
  ofs += move;
  buf += move;
  MACH_WRITE_UINT32(buf, table_id_);
  move = sizeof(uint32_t);
  ofs += move;
  buf += move;
  MACH_WRITE_UINT32(buf, key_map_.size());
  move = sizeof(uint32_t);
  ofs += move;
  buf += move;
  for (auto i : key_map_) {
    MACH_WRITE_UINT32(buf, i);
    move = sizeof(uint32_t);
    ofs += move;
    buf += move;
  }
  return ofs;
}

uint32_t IndexMetadata::GetSerializedSize() const {
  uint32_t ofs = 0;
  ofs += 5 * sizeof(uint32_t);
  ofs += MACH_STR_SERIALIZED_SIZE(index_name_);
  ofs += sizeof(uint32_t) * key_map_.size();
  return ofs;
}

uint32_t IndexMetadata::DeserializeFrom(char *buf, IndexMetadata *&index_meta, MemHeap *heap) {
  uint32_t ofs = 0, move;
  uint32_t magic_num = MACH_READ_UINT32(buf);
  if (magic_num != index_meta->INDEX_METADATA_MAGIC_NUM)
    LOG(ERROR) << "INDEX_METADATA_MAGIC_NUM doesn't match!" << std::endl;
  move = sizeof(uint32_t);
  buf += move;
  ofs += move;
  uint32_t index_id = MACH_READ_UINT32(buf);
  move = sizeof(uint32_t);
  buf += move;
  ofs += move;
  uint32_t len = MACH_READ_UINT32(buf);
  move = sizeof(uint32_t);
  ofs += move;
  buf += move;
  char index_name[len + 1];
  strncpy(index_name, buf, len);
  index_name[len] = '\0';
  move = len + 4;
  ofs += move;
  buf += move;
  uint32_t table_id = MACH_READ_UINT32(buf);
  move = sizeof(uint32_t);
  buf += move;
  ofs += move;
  uint32_t map_size = MACH_READ_UINT32(buf);
  ofs += move;
  buf += move;

  std::vector<uint32_t> key_map;

  for (auto i = 0; i < map_size; i++) {
    key_map.push_back(MACH_READ_UINT32(buf));
    move = sizeof(uint32_t);
    ofs += move;
    buf += move;
  }
  index_meta = new IndexMetadata(index_id, index_name, table_id, key_map);
  return ofs;
}