#include "record/row.h"
#include "glog/logging.h"

//TODO:schema参数没用上
uint32_t Row::SerializeTo(char *buf, Schema *schema) const {
  // replace with your code here
  uint32_t ofs = 0,move;
  for(auto field:fields_){
    move=field->SerializeTo(buf);
    ofs+=move;
    buf+=move;
  }
  return ofs;
}

uint32_t Row::DeserializeFrom(char *buf, Schema *schema) {
  // replace with your code here
  fields_.clear();
  uint32_t ofs = 0,move;
  for(size_t i=0;i<schema->GetColumnCount();i++){
    TypeId type = schema->GetColumn(i)->GetType();
    Field *field = new Field(type);
    move=field->DeserializeFrom(buf,type,&field,false,heap_);
    ofs+=move;
    buf+=move;
    fields_.push_back(field);
  }
  return ofs;
}

//TODO: schema参数没用上
uint32_t Row::GetSerializedSize(Schema *schema) const {
  // replace with your code here
  uint32_t ofs = 0;
  for(auto field:this->fields_)
    ofs+=field->GetSerializedSize();
  return ofs;
}
