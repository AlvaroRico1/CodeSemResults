bool EpsCopyOutputStream::Skip(int count, uint8_t** pp) {
  if (count < 0) return false;
  if (had_error_) {
    *pp = buffer_;
    return false;
  }
  int size = Flush(*pp);
  if (had_error_) {
    *pp = buffer_;
    return false;
  }
  void* data = buffer_end_;
  while (count > size) {
    count -= size;
    if (!stream_->Next(&data, &size)) {
      *pp = Error();
      return false;
    }
  }
  *pp = SetInitialBuffer(static_cast<uint8_t*>(data) + count, size - count);
  return true;
}


// Source: coded_stream.cc
// Lines 722-743
