void GrowingArrayByteSink::Expand(size_t amount) {  // Expand by at least 50%.
  size_t new_capacity = std::max(capacity_ + amount, (3 * capacity_) / 2);
  char* bigger = new char[new_capacity];
  memcpy(bigger, buf_, size_);
  delete[] buf_;
  buf_ = bigger;
  capacity_ = new_capacity;
}


// Source: bytestream.cc
// Lines 123-130
