bool CodedInputStream::ReadLittleEndian64Fallback(uint64_t* value) {
  uint8_t bytes[sizeof(*value)];

  const uint8_t* ptr;
  if (BufferSize() >= static_cast<int64_t>(sizeof(*value))) {
    // Fast path:  Enough bytes in the buffer to read directly.
    ptr = buffer_;
    Advance(sizeof(*value));
  } else {
    // Slow path:  Had to read past the end of the buffer.
    if (!ReadRaw(bytes, sizeof(*value))) return false;
    ptr = bytes;
  }
  ReadLittleEndian64FromArray(ptr, value);
  return true;
}


// Source: coded_stream.cc
// Lines 329-344
