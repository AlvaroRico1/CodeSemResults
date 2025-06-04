bool MessageLite::SerializePartialToArray(void* data, int size) const {
  const size_t byte_size = ByteSizeLong();
  if (byte_size > INT_MAX) {
    GOOGLE_LOG(ERROR) << GetTypeName()
               << " exceeded maximum protobuf size of 2GB: " << byte_size;
    return false;
  }
  if (size < static_cast<int64_t>(byte_size)) return false;
  uint8_t* start = reinterpret_cast<uint8_t*>(data);
  SerializeToArrayImpl(*this, start, byte_size);
  return true;
}


// Source: message_lite.cc
// Lines 476-487
