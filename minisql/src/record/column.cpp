#include "record/column.h"
#include "glog/logging.h"

Column::Column(std::string column_name, TypeId type, uint32_t index, bool nullable, bool unique)
        : name_(std::move(column_name)), type_(type), table_ind_(index),
          nullable_(nullable), unique_(unique) {
  // ASSERT(type != TypeId::kTypeChar, "Wrong constructor for CHAR type.");
  switch (type) {
    case TypeId::kTypeInt :
      len_ = sizeof(int32_t);
      break;
    case TypeId::kTypeFloat :
      len_ = sizeof(float_t);
      break;
    //之前没有kTypeChar的情况，现在添加如下：
    case TypeId::kTypeChar :
      len_ = name_.length()+4;
      break;
    default:
      ASSERT(false, "Unsupported column type.");
  }
}

Column::Column(std::string column_name, TypeId type, uint32_t length, uint32_t index, bool nullable, bool unique)
        : name_(std::move(column_name)), type_(type), len_(length),
          table_ind_(index), nullable_(nullable), unique_(unique) {
  ASSERT(type == TypeId::kTypeChar, "Wrong constructor for non-VARCHAR type.");
}

Column::Column(const Column *other) : name_(other->name_), type_(other->type_), len_(other->len_),
                                      table_ind_(other->table_ind_), nullable_(other->nullable_),
                                      unique_(other->unique_) {}

uint32_t Column::SerializeTo(char *buf) const {
  // replace with your code here
  uint32_t ofs = 0,move;
  MACH_WRITE_UINT32(buf,this->COLUMN_MAGIC_NUM);  //write attribute of column into buf: magic number, name  
  move = sizeof(uint32_t);                          //type, index, nullable flag, unique flag
  buf += move;ofs += move;
  MACH_WRITE_UINT32(buf,this->name_.length());
  move = sizeof(uint32_t);
  buf += move;ofs += move;
  MACH_WRITE_STRING(buf,this->name_);
  move = name_.length()+4;
  buf += move;ofs += move;
  MACH_WRITE_TO(TypeId,buf,this->type_);
  move = sizeof(this->type_);
  buf += move;ofs += move;
  MACH_WRITE_UINT32(buf,this->table_ind_);
  move = sizeof(uint32_t);
  buf += move;ofs += move; 
  MACH_WRITE_TO(bool,buf,this->nullable_);
  move = sizeof(bool);
  buf += move;ofs += move;
  MACH_WRITE_TO(bool,buf,this->unique_);
  move = sizeof(bool);
  buf += move;ofs += move;
  return ofs; //return the offset of buf while serializing
}

uint32_t Column::GetSerializedSize() const {
  // replace with your code here
  return 3*sizeof(uint32_t)+2*sizeof(bool)+MACH_STR_SERIALIZED_SIZE(this->name_)+sizeof(this->type_); //return the offset of buf while serializing
}

uint32_t Column::DeserializeFrom(char *buf, Column *&column, MemHeap *heap) {
  // replace with your code here
  if(column != nullptr){
    LOG(WARNING)<<"Pointer to column is not null in column deserialize."<<std::endl;
  }
  uint32_t ofs = 0,move;
  uint32_t magic_num = MACH_READ_UINT32(buf);               //read attribute of column from buf: magic number, name  
  move = sizeof(magic_num);                                 //type, index, nullable flag, unique flag
  buf += move;ofs += move;
  if(magic_num!=column->COLUMN_MAGIC_NUM)                     //check the read in magic_num
    LOG(ERROR)<<"COLUMN_MAGIC_NUM doesn't match!"<<std::endl; //this means the read in data doesn't accord with expectation
  uint32_t len = MACH_READ_UINT32(buf);
  move = sizeof(uint32_t);
  buf += move;ofs += move;
  char name[len+1];
  strncpy(name,buf,len);
  name[len] = '\0';
  move = len+4;
  buf += move;ofs += move;
  TypeId type = MACH_READ_FROM(TypeId,buf);
  move = sizeof(type);
  buf += move;ofs += move;
  uint32_t index = MACH_READ_UINT32(buf);
  move = sizeof(uint32_t);
  buf += move;ofs += move;
  bool nullable = MACH_READ_FROM(bool,buf);
  move = sizeof(bool);
  buf += move;ofs += move;
  bool unique = MACH_READ_FROM(bool,buf);
  move = sizeof(bool);
  buf += move;ofs += move;
  column = ALLOC_P(heap,Column)(name,type,index,nullable,unique);
  return ofs;
}
bool Column::IsUnique() { return unique_; }
