static inline float float4get(const uchar *M) {
  float V;
  memcpy(&V, (M), sizeof(float));
  return V;
}


// Source: little_endian.h
// Lines 94-98
