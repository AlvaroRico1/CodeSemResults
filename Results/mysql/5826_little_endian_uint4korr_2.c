static inline uint32 uint4korr(const uchar *A) {
  uint32 ret;
  memcpy(&ret, A, sizeof(ret));
  return ret;
}


// Source: little_endian.h
// Lines 66-70
