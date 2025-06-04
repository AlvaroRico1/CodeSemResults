static inline uint16 uint2korr(const uchar *A) {
  uint16 ret;
  memcpy(&ret, A, sizeof(ret));
  return ret;
}


// Source: little_endian.h
// Lines 60-64
