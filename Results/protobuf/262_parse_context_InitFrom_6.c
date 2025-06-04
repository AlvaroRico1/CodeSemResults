const char* EpsCopyInputStream::InitFrom(io::ZeroCopyInputStream* zcis) {
  zcis_ = zcis;
  const void* data;
  int size;
  limit_ = INT_MAX;
  if (zcis->Next(&data, &size)) {
    overall_limit_ -= size;
    if (size > kSlopBytes) {
      auto ptr = static_cast<const char*>(data);
      limit_ -= size - kSlopBytes;
      limit_end_ = buffer_end_ = ptr + size - kSlopBytes;
      next_chunk_ = buffer_;
      if (aliasing_ == kOnPatch) aliasing_ = kNoDelta;
      return ptr;
    } else {
      limit_end_ = buffer_end_ = buffer_ + kSlopBytes;
      next_chunk_ = buffer_;
      auto ptr = buffer_ + 2 * kSlopBytes - size;
      std::memcpy(ptr, data, size);
      return ptr;
    }


// Source: parse_context.cc
// Lines 250-270
