#include "catalog/catalog.h"

void CatalogMeta::SerializeTo(char *buf) const {
  MACH_WRITE_UINT32(buf, CATALOG_METADATA_MAGIC_NUM);
  buf += sizeof(uint32_t);
  MACH_WRITE_INT32(buf, table_meta_pages_.size());
  buf += sizeof(int32_t);
  for (auto iter = table_meta_pages_.begin(); iter != table_meta_pages_.end(); iter++) {
    MACH_WRITE_UINT32(buf, iter->first);
    buf += sizeof(uint32_t);
    MACH_WRITE_INT32(buf, iter->second);
    buf += sizeof(int32_t);
  }
  MACH_WRITE_INT32(buf, index_meta_pages_.size());
  buf += sizeof(int32_t);
  for (auto iter = index_meta_pages_.begin(); iter != index_meta_pages_.end(); iter++) {
    MACH_WRITE_UINT32(buf, iter->first);
    buf += sizeof(uint32_t);
    MACH_WRITE_INT32(buf, iter->second);
    buf += sizeof(int32_t);
  }
  // ASSERT(false, "Not Implemented yet");
}

CatalogMeta *CatalogMeta::DeserializeFrom(char *buf, MemHeap *heap) {
  CatalogMeta *meta = CatalogMeta::NewInstance(heap);
  uint32_t magic_num = MACH_READ_UINT32(buf);
  buf += sizeof(uint32_t);
  if (magic_num != meta->CATALOG_METADATA_MAGIC_NUM)
    LOG(ERROR) << "CATALOG_METADATA_MAGIC_NUM doesn't match!" << std::endl;
  const int table_nums = MACH_READ_INT32(buf);
  buf += sizeof(int32_t);
  for (auto i = 0; i < table_nums; i++) {
    uint32_t first = MACH_READ_UINT32(buf);
    buf += sizeof(uint32_t);
    int32_t second = MACH_READ_INT32(buf);
    buf += sizeof(int32_t);
    meta->table_meta_pages_.emplace(first, second);
  }
  const int index_nums = MACH_READ_INT32(buf);
  buf += sizeof(int32_t);
  for (auto i = 0; i < index_nums; i++) {
    uint32_t first = MACH_READ_UINT32(buf);
    buf += sizeof(uint32_t);
    int32_t second = MACH_READ_INT32(buf);
    buf += sizeof(int32_t);
    meta->index_meta_pages_.emplace(first, second);
  }
  // ASSERT(false, "Not Implemented yet");
  return meta;
}

uint32_t CatalogMeta::GetSerializedSize() const {
  uint32_t buf = 0;
  buf += 2 * sizeof(int32_t);
  buf += sizeof(uint32_t);
  for (auto iter = table_meta_pages_.begin(); iter != table_meta_pages_.end(); iter++)
    buf += sizeof(uint32_t) + sizeof(int32_t);
  for (auto iter = index_meta_pages_.begin(); iter != index_meta_pages_.end(); iter++)
    buf += sizeof(uint32_t) + sizeof(int32_t);
  // ASSERT(false, "Not Implemented yet");
  return buf;
}

CatalogMeta::CatalogMeta() {}

/**
 * @brief 在数据库实例初次创建时，初始化元信息。
 * 后续重新打开数据库实例时，从数据库文件中加载所有表和索引信息。
 * 构建TableInfo和IndexInfo信息置于内存中
 */
CatalogManager::CatalogManager(BufferPoolManager *buffer_pool_manager, LockManager *lock_manager,
                               LogManager *log_manager, bool init)
    : buffer_pool_manager_(buffer_pool_manager),
      lock_manager_(lock_manager),
      log_manager_(log_manager),
      heap_(new SimpleMemHeap()) {
  // ASSERT(false, "Not Implemented yet");
  // 在数据库实例(DBStorageEngine)初次创建时(init = true) 初始化元信息
  if (init) {
    catalog_meta_ = CatalogMeta::NewInstance(heap_);
    next_index_id_ = 0;
    next_table_id_ = 0;
  } else {
    // 在后续重新打开数据库实例时，从数据库文件中加载所有表和索引信息
    Page *meta_page = buffer_pool_manager->FetchPage(CATALOG_META_PAGE_ID);
    catalog_meta_ = CatalogMeta::NewInstance(heap_);
    catalog_meta_ = CatalogMeta::DeserializeFrom(meta_page->GetData(), heap_);
    // 构建TableInfo和IndexInfo信息置于内存中
    for (auto iter = catalog_meta_->table_meta_pages_.begin(); iter != catalog_meta_->table_meta_pages_.end(); iter++) {
      LoadTable(iter->first, iter->second);
      next_table_id_ = (iter->first) + 1;
    }
    for (auto iter = catalog_meta_->index_meta_pages_.begin(); iter != catalog_meta_->index_meta_pages_.end(); iter++) {
      LoadIndex(iter->first, iter->second);
      next_index_id_ = (iter->first) + 1;
    }
    buffer_pool_manager->UnpinPage(CATALOG_META_PAGE_ID, false);
    FlushCatalogMetaPage();
  }
}

CatalogManager::~CatalogManager() { delete heap_; }

/**
 * @brief 创建Table并将内容序列化到内存
 * @return 返回函数执行的状态信息dberr_t类型
 */
dberr_t CatalogManager::CreateTable(const string &table_name, TableSchema *schema, Transaction *txn,
                                    TableInfo *&table_info) {
  // ASSERT(false, "Not Implemented yet");
  if (table_names_.find(table_name) != table_names_.end()) return DB_TABLE_ALREADY_EXIST;

  // create table heap
  TableHeap *table_heap = TableHeap::Create(buffer_pool_manager_, schema, txn, log_manager_, lock_manager_, heap_);

  // create table meta and set root_page_id to previous table heap page
  page_id_t table_meta_page_id;
  buffer_pool_manager_->NewPage(table_meta_page_id);

  // page->Init(table_meta_page_id, INVALID_PAGE_ID, nullptr, nullptr);
  TableMetadata *table_meta =
      TableMetadata::Create(next_table_id_, table_name, table_heap->GetFirstPageId(), schema, heap_);

  // create table_info = table_meta+table_heap
  TableInfo *table_info_ = TableInfo::Create(heap_);
  table_info_->Init(table_meta, table_heap);

  // write table_meta to page(table_meta_page_id)
  Page *table_meta_page = buffer_pool_manager_->FetchPage(table_meta_page_id);
  table_meta->SerializeTo(table_meta_page->GetData());
  buffer_pool_manager_->UnpinPage(table_meta_page_id, true);
  buffer_pool_manager_->UnpinPage(table_meta_page_id, true);

  // update table_meta_pages_ in catalog_meta_ and flush
  catalog_meta_->table_meta_pages_.emplace(next_table_id_, table_meta_page_id);
  Page *catalog_meta_page = buffer_pool_manager_->FetchPage(CATALOG_META_PAGE_ID);
  catalog_meta_->SerializeTo(catalog_meta_page->GetData());
  buffer_pool_manager_->UnpinPage(CATALOG_META_PAGE_ID, true);

  // update CatalogManager
  table_names_.emplace(table_name, next_table_id_);
  tables_.emplace(next_table_id_, table_info_);
  next_table_id_ += 1;

  // prepare to return
  table_info = table_info_;
  return DB_SUCCESS;
}

dberr_t CatalogManager::GetTable(const string &table_name, TableInfo *&table_info) {
  // ASSERT(false, "Not Implemented yet");
  if (table_names_.empty()) return DB_TABLE_NOT_EXIST;
  if (table_names_.end() == table_names_.find(table_name))  // not exit table whose name=table_name_
    return DB_TABLE_NOT_EXIST;
  else
    table_info = tables_[table_names_[table_name]];  // point table_info to the TableInfo with name table_name_
  return DB_SUCCESS;
}

dberr_t CatalogManager::GetTables(vector<TableInfo *> &tables) const {
  // ASSERT(false, "Not Implemented yet");
  if (tables_.size() == 0) return DB_TABLE_NOT_EXIST;
  for (auto iter = tables_.begin(); iter != tables_.end(); iter++) tables.push_back(iter->second);
  return DB_SUCCESS;
}

dberr_t CatalogManager::CreateIndex(const std::string &table_name, const string &index_name,
                                    const std::vector<std::string> &index_keys, Transaction *txn,
                                    IndexInfo *&index_info) {
  // ASSERT(false, "Not Implemented yet");
  // 查找是否存在table_name
  if (table_names_.find(table_name) == table_names_.end()) return DB_TABLE_NOT_EXIST;

  // create indexinfo
  // 1. get tableinfo
  table_id_t table_id = table_names_[table_name];
  TableInfo *table_info = tables_.find(table_id)->second;
  // 2. get mapping of index_keys to key_map
  vector<uint32_t> key_map;

  std::vector<Column *> columns = table_info->GetSchema()->GetColumns();
  for (auto i = 0; i < index_keys.size(); i++) {
    for (auto j = 0; j < columns.size(); j++) {
      if ((columns[j]->GetName()) == index_keys[i]) {
        key_map.push_back(j);
        break;
      }
      if (j == columns.size() - 1) return DB_COLUMN_NAME_NOT_EXIST;
    }
  }

  // 3.在index_names中查找是否存在当前table_name的索引
  auto it = index_names_.find(table_name);
  if (it != index_names_.end()) {
    auto iter = it->second.find(index_name);
    if (iter != it->second.end()) return DB_INDEX_ALREADY_EXIST;
  } else {
    std::unordered_map<std::string, index_id_t> indexes;
    index_names_.emplace(table_name, indexes);
    it = index_names_.find(table_name);
  }

  IndexMetadata *index_meta = IndexMetadata::Create(next_index_id_, index_name, table_id, key_map, heap_);
  IndexInfo *index_info_ = IndexInfo::Create(heap_);
  index_info_->Init(index_meta, table_info, buffer_pool_manager_);
  index_info = index_info_;

  page_id_t page_id;
  Page *index_info_page = buffer_pool_manager_->NewPage(page_id);
  index_meta->SerializeTo(index_info_page->GetData());

  // 4. emplace into index_names_,indexes_
  it->second.emplace(index_name, next_index_id_);
  indexes_.emplace(next_index_id_, index_info_);
  // 5. emplace into catalog_meta_
  catalog_meta_->index_meta_pages_.emplace(next_index_id_, page_id);
  Page *catalog_meta_page = buffer_pool_manager_->FetchPage(CATALOG_META_PAGE_ID);
  catalog_meta_->SerializeTo(catalog_meta_page->GetData());

  buffer_pool_manager_->UnpinPage(CATALOG_META_PAGE_ID, true);
  buffer_pool_manager_->UnpinPage(page_id, true);

  next_index_id_ += 1;
  return DB_SUCCESS;
}

dberr_t CatalogManager::GetIndex(const std::string &table_name, const std::string &index_name,
                                 IndexInfo *&index_info) const {
  // ASSERT(false, "Not Implemented yet");
  auto it = index_names_.find(table_name);
  if (it == index_names_.end()) return DB_INDEX_NOT_FOUND;
  auto iter = it->second.find(index_name);
  if (iter == it->second.end()) return DB_INDEX_NOT_FOUND;
  index_info = indexes_.find(iter->second)->second;
  return DB_SUCCESS;
}

dberr_t CatalogManager::GetTableIndexes(const std::string &table_name, std::vector<IndexInfo *> &indexes) const {
  // ASSERT(false, "Not Implemented yet");
  auto it = index_names_.find(table_name);
  if (it == index_names_.end()) return DB_INDEX_NOT_FOUND;
  for (auto index_id : it->second) indexes.push_back(indexes_.find(index_id.second)->second);
  return DB_SUCCESS;
}

dberr_t CatalogManager::DropTable(const string &table_name) {
  // ASSERT(false, "Not Implemented yet");
  if (table_names_.find(table_name) == table_names_.end())  // 如果不存在该名字的table
    return DB_TABLE_NOT_EXIST;
  
  TableInfo *table_info = tables_[table_names_[table_name]];
  tables_.erase(table_names_[table_name]);                           // 先删除tables_图中的数据
  catalog_meta_->table_meta_pages_.erase(table_names_[table_name]);  // 再删除catalog_meta_中的数据
  table_names_.erase(table_name);                                    // 最后从table_names_中删除数据
  
  //delete table heap, table meta in page
  table_info->GetTableHeap()->FreeHeap();
  table_info->GetMemHeap()->Free(table_info);
  
  // delete table_info;
  
  //update catalog_meta in CATALOG_META_PAGE
  Page *catalog_meta_page = buffer_pool_manager_->FetchPage(CATALOG_META_PAGE_ID);
  catalog_meta_->SerializeTo(catalog_meta_page->GetData());
  buffer_pool_manager_->UnpinPage(CATALOG_META_PAGE_ID, true);

  return DB_SUCCESS;
}

dberr_t CatalogManager::DropIndex(const string &table_name, const string &index_name) {
  // ASSERT(false, "Not Implemented yet");
  auto it = index_names_.find(table_name);
  if (it == index_names_.end()) return DB_INDEX_NOT_FOUND;
  auto iter = it->second.find(index_name);
  if (iter == it->second.end()) return DB_INDEX_NOT_FOUND;

  IndexInfo *index_info = indexes_[iter->second];
  
  indexes_.erase(iter->second);
  catalog_meta_->index_meta_pages_.erase(iter->second);
  it->second.erase(iter->first);
  
  index_info->GetIndex()->Destroy();
  index_info->GetMemHeap()->Free(index_info);
  // delete index_info;

  Page *catalog_meta_page = buffer_pool_manager_->FetchPage(CATALOG_META_PAGE_ID);
  catalog_meta_->SerializeTo(catalog_meta_page->GetData());
  buffer_pool_manager_->UnpinPage(CATALOG_META_PAGE_ID, true);

  return DB_SUCCESS;
}

dberr_t CatalogManager::FlushCatalogMetaPage() const {
  // ASSERT(false, "Not Implemented yet");
  buffer_pool_manager_->FlushPage(CATALOG_META_PAGE_ID);
  return DB_SUCCESS;
}

dberr_t CatalogManager::LoadTable(const table_id_t table_id, const page_id_t page_id) {
  // ASSERT(false, "Not Implemented yet");
  Page *table_meta_page = buffer_pool_manager_->FetchPage(page_id);
  TableMetadata *table_meta;
  TableMetadata::DeserializeFrom(table_meta_page->GetData(), table_meta, heap_);
  TableInfo *table_info;
  table_info = TableInfo::Create(heap_);
  TableHeap *table_heap;
  table_heap = TableHeap::Create(buffer_pool_manager_, table_meta->GetFirstPageId(), table_meta->GetSchema(),
                                 log_manager_, lock_manager_, heap_);

  std::string table_name = table_meta->GetTableName();

  table_info->Init(table_meta, table_heap);
  table_names_.emplace(table_name, table_id);
  tables_.emplace(table_id, table_info);
  buffer_pool_manager_->UnpinPage(page_id, false);
  return DB_SUCCESS;
}

dberr_t CatalogManager::LoadIndex(const index_id_t index_id, const page_id_t page_id) {
  // ASSERT(false, "Not Implemented yet");
  Page *index_page = buffer_pool_manager_->FetchPage(page_id);
  IndexMetadata *index_meta;
  IndexMetadata::DeserializeFrom(index_page->GetData(), index_meta, heap_);
  TableInfo *table_info = tables_[index_meta->GetTableId()];
  IndexInfo *index_info;
  index_info = IndexInfo::Create(heap_);
  index_info->Init(index_meta, table_info, buffer_pool_manager_);
  // 查table_name是否存在
  std::string table_name = index_info->GetTableInfo()->GetTableName();
  std::string index_name = index_info->GetIndexName();
  if (index_names_.find(table_name) == index_names_.end()) {
    std::unordered_map<std::string, index_id_t> indexes;
    index_names_.emplace(table_name, indexes);
  }
  auto iter = index_names_.find(table_name);
  iter->second.emplace(index_name, index_id);
  indexes_.emplace(index_id, index_info);
  buffer_pool_manager_->UnpinPage(page_id, false);
  return DB_SUCCESS;
}

dberr_t CatalogManager::GetTable(const table_id_t table_id, TableInfo *&table_info) {
  // ASSERT(false, "Not Implemented yet");
  if (tables_.empty()) return DB_TABLE_NOT_EXIST;
  if (tables_.find(table_id) == tables_.end())
    return DB_TABLE_NOT_EXIST;
  else
    table_info = tables_[table_id];
  return DB_SUCCESS;
}