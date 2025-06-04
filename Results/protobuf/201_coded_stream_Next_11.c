uint8_t* EpsCopyOutputStream::Next() {
  GOOGLE_DCHECK(!had_error_);  // NOLINT
  if (PROTOBUF_PREDICT_FALSE(stream_ == nullptr)) return Error();
  if (buffer_end_) {
    // We're in the patch buffer and need to fill up the previous buffer.
    std::memcpy(buffer_end_, buffer_, end_ - buffer_);
    uint8_t* ptr;
    int size;
    do {
      void* data;
      if (PROTOBUF_PREDICT_FALSE(!stream_->Next(&data, &size))) {
        // Stream has an error, we use the patch buffer to continue to be
        // able to write.
        return Error();
      }
      ptr = static_cast<uint8_t*>(data);
    } while (size == 0);


// Source: coded_stream.cc
// Lines 788-804
