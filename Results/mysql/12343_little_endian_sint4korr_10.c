static inline int32 sint4korr(const uchar *A) {
  int32 ret;
  memcpy(&ret, A, sizeof(ret));
  return ret;
}


// Source: little_endian.h
// Lines 54-58
