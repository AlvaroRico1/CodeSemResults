void Printer::CopyToBuffer(const char* data, int size) {
  if (failed_) return;
  if (size == 0) return;

  while (size > buffer_size_) {
    // Data exceeds space in the buffer.  Copy what we can and request a
    // new buffer.
    if (buffer_size_ > 0) {
      memcpy(buffer_, data, buffer_size_);
      offset_ += buffer_size_;
      data += buffer_size_;
      size -= buffer_size_;
    }
    void* void_buffer;
    failed_ = !output_->Next(&void_buffer, &buffer_size_);
    if (failed_) return;
    buffer_ = reinterpret_cast<char*>(void_buffer);
  }


// Source: printer.cc
// Lines 247-264
