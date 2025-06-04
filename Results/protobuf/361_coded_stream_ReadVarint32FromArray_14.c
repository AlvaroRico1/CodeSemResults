inline ::std::pair<bool, const uint8_t*> ReadVarint32FromArray(
    uint32_t first_byte, const uint8_t* buffer, uint32_t* value) {
  // Fast path:  We have enough bytes left in the buffer to guarantee that
  // this read won't cross the end, so we can skip the checks.
  GOOGLE_DCHECK_EQ(*buffer, first_byte);
  GOOGLE_DCHECK_EQ(first_byte & 0x80, 0x80) << first_byte;
  const uint8_t* ptr = buffer;
  uint32_t b;
  uint32_t result = first_byte - 0x80;
  ++ptr;  // We just processed the first byte.  Move on to the second.
  b = *(ptr++);
  result += b << 7;
  if (!(b & 0x80)) goto done;
  result -= 0x80 << 7;
  b = *(ptr++);
  result += b << 14;
  if (!(b & 0x80)) goto done;
  result -= 0x80 << 14;
  b = *(ptr++);
  result += b << 21;
  if (!(b & 0x80)) goto done;
  result -= 0x80 << 21;
  b = *(ptr++);
  result += b << 28;
  if (!(b & 0x80)) goto done;
  // "result -= 0x80 << 28" is irrevelant.

  // If the input is larger than 32 bits, we still need to read it all
  // and discard the high-order bits.
  for (int i = 0; i < kMaxVarintBytes - kMaxVarint32Bytes; i++) {
    b = *(ptr++);
    if (!(b & 0x80)) goto done;
  }

  // We have overrun the maximum size of a varint (10 bytes).  Assume
  // the data is corrupt.
  return std::make_pair(false, ptr);

done:
  *value = result;
  return std::make_pair(true, ptr);
}


// Source: coded_stream.cc
// Lines 370-411
