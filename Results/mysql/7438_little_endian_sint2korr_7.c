static inline int16 sint2korr(const uchar *A) {
  int16 ret;
  memcpy(&ret, A, sizeof(ret));
  return ret;
}


// Source: little_endian.h
// Lines 48-52
