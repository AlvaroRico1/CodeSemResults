static inline double float8get(const uchar *M) {
  double V;
  memcpy(&V, M, sizeof(double));
  return V;
}


// Source: little_endian.h
// Lines 104-108
