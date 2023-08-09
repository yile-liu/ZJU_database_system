#include "catalog/table.h"

uint32_t TableMetadata::SerializeTo(char *buf) const {
  uint32_t ofs = 0, move;
  MACH_WRITE_UINT32(buf, TABLE_METADATA_MAGIC_NUM);
  move = sizeof(uint32_t);
  ofs += move;
  buf += move;
  MACH_WRITE_UINT32(buf, table_id_);
  move = sizeof(uint32_t);
  ofs += move;
  buf += move;
  MACH_WRITE_UINT32(buf,table_name_.length());
  move = sizeof(uint32_t);
  ofs += move;
  buf += move;
  MACH_WRITE_STRING(buf, table_name_);
  move = MACH_STR_SERIALIZED_SIZE(table_name_);
  ofs += move;
  buf += move;
  MACH_WRITE_INT32(buf, root_page_id_);
  move = sizeof(int32_t);
  ofs += move;
  buf += move;
  move = schema_->SerializeTo(buf);
  ofs += move;
  return ofs;
}

uint32_t TableMetadata::GetSerializedSize() const {
  uint32_t ofs = 0;
  ofs += 3 * sizeof(uint32_t);
  ofs += MACH_STR_SERIALIZED_SIZE(table_name_);
  ofs += sizeof(int32_t);
  ofs += schema_->GetSerializedSize();
  return ofs;
}

/**
 * @param heap Memory heap passed by TableInfo
 */
uint32_t TableMetadata::DeserializeFrom(char *buf, TableMetadata *&table_meta, MemHeap *heap) {
  uint32_t ofs = 0, move;
  uint32_t magic_num = MACH_READ_UINT32(buf);
  if (magic_num != TABLE_METADATA_MAGIC_NUM) LOG(ERROR) << "TABLE_METADATA_MAGIC_NUM dosen't match!" << std::endl;
  move = sizeof(uint32_t);
  ofs += move;
  buf += move;
  table_id_t table_id;
  table_id = MACH_READ_UINT32(buf);
  move = sizeof(uint32_t);
  ofs += move;
  buf += move;
  uint32_t len = MACH_READ_UINT32(buf);
  move = sizeof(uint32_t);
  ofs += move;
  buf += move;
  // table_meta->table_name_ = MACH_READ_FROM(std::string, buf);
  char table_name[len+1];
  // memcpy((char*)(table_name_.c_str()),buf,len);
  strncpy((char*)table_name,buf,len);
  table_name[len]='\0';
  move = len+4;
  ofs += move;
  buf += move;
  page_id_t root_page_id;
  root_page_id = MACH_READ_INT32(buf);
  move = sizeof(int32_t);
  ofs += move;
  buf += move;
  Schema *schema = nullptr;
  move = Schema::DeserializeFrom(buf, schema, heap);
  ofs += move;
  table_meta = new TableMetadata(table_id,table_name,root_page_id,schema);
  return ofs;
}

/**
 * Only called by create table
 *
 * @param heap Memory heap passed by TableInfo
 */
TableMetadata *TableMetadata::Create(table_id_t table_id, std::string table_name, page_id_t root_page_id,
                                     TableSchema *schema, MemHeap *heap) {
  // allocate space for table metadata
  void *buf = heap->Allocate(sizeof(TableMetadata));
  return new (buf) TableMetadata(table_id, table_name, root_page_id, schema);
}

TableMetadata::TableMetadata(table_id_t table_id, std::string table_name, page_id_t root_page_id, TableSchema *schema)
    : table_id_(table_id), table_name_(table_name), root_page_id_(root_page_id), schema_(schema) {}
