static inline longlong sint8korr(const uchar *A) {
  longlong ret;
  memcpy(&ret, A, sizeof(ret));
  return ret;
}


// Source: little_endian.h
// Lines 78-82
