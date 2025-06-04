Message* DynamicMessage::New(Arena* arena) const {
  if (arena != nullptr) {
    void* new_base = Arena::CreateArray<char>(arena, type_info_->size);
    memset(new_base, 0, type_info_->size);
    return new (new_base) DynamicMessage(type_info_, arena);
  } else {
    void* new_base = operator new(type_info_->size);
    memset(new_base, 0, type_info_->size);
    return new (new_base) DynamicMessage(type_info_);
  }


// Source: dynamic_message.cc
// Lines 614-623
