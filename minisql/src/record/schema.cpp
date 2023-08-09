#include "record/schema.h"
#include "glog/logging.h"

uint32_t Schema::SerializeTo(char *buf) const {
  // replace with your code here
  uint32_t ofs = 0,move;
  MACH_WRITE_INT32(buf,this->SCHEMA_MAGIC_NUM);   //write magic number of schema into buf
  move = sizeof(uint32_t);
  ofs += move;buf += move;
  //把columns_的大小先写入内存
  MACH_WRITE_UINT32(buf,columns_.size());
  move = sizeof(uint32_t);
  ofs += move;buf += move;
  for(auto column:this->columns_){
    move = column->SerializeTo(buf);
    ofs += move;buf += move;
  }
  return ofs;
}

uint32_t Schema::GetSerializedSize() const {
  // replace with your code here
  uint32_t ofs = 0;
  ofs += 2*sizeof(uint32_t);
  for(auto column:this->columns_)
    ofs+=column->GetSerializedSize();
  return ofs;
}

uint32_t Schema::DeserializeFrom(char *buf, Schema *&schema, MemHeap *heap) {
  // replace with your code here
  uint32_t ofs = 0,move;
  uint32_t magic_num = MACH_READ_UINT32(buf);
  move = sizeof(uint32_t);
  ofs += move;buf += move; 
  if(magic_num!=schema->SCHEMA_MAGIC_NUM)                     //check the schema magic number
    LOG(ERROR)<<"SCHEMA_MAGIC_NUM doesn't match!"<<std::endl;
  uint32_t size = MACH_READ_UINT32(buf);
  move = sizeof(uint32_t);
  ofs += move;buf += move;
  std::vector<Column*> columns_;
  for(auto i=0;i<size;i++){
    Column* column = nullptr;
    move = Column::DeserializeFrom(buf,column,heap);
    columns_.push_back(column);
    ofs += move;buf += move;
  }
  schema = new Schema(columns_);
  return ofs;
}