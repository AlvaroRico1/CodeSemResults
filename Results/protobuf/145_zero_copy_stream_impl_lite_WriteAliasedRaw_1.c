bool CopyingOutputStreamAdaptor::WriteAliasedRaw(const void* data, int size) {
  if (size >= buffer_size_) {
    if (!Flush() || !copying_stream_->Write(data, size)) {
      return false;
    }
    GOOGLE_DCHECK_EQ(buffer_used_, 0);
    position_ += size;
    return true;
  }

  void* out;
  int out_size;
  while (true) {
    if (!Next(&out, &out_size)) {
      return false;
    }

    if (size <= out_size) {
      std::memcpy(out, data, size);
      BackUp(out_size - size);
      return true;
    }

    std::memcpy(out, data, out_size);
    data = static_cast<const char*>(data) + out_size;
    size -= out_size;
  }
  return true;
}


// Source: zero_copy_stream_impl_lite.cc
// Lines 348-376
