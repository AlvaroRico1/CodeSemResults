static inline const uchar *skip_trailing_space(const uchar *ptr, size_t len) {
  const uchar *end = ptr + len;
  while (end - ptr >= 8) {
    uint64_t chunk;
    memcpy(&chunk, end - 8, sizeof(chunk));
    if (chunk != 0x2020202020202020ULL) break;
    end -= 8;
  }
  while (end > ptr && end[-1] == 0x20) end--;
  return (end);
}


// Source: m_string.h
// Lines 328-338
