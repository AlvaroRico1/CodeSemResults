static void my_hash_sort_bin(const CHARSET_INFO *cs MY_ATTRIBUTE((unused)),
                             const uchar *key, size_t len, uint64 *nr1,
                             uint64 *nr2) {
  const uchar *pos = key;
  uint64 tmp1;
  uint64 tmp2;

  key += len;

  tmp1 = *nr1;
  tmp2 = *nr2;

  for (; pos < key; pos++) {
    tmp1 ^= (uint64)((((uint)tmp1 & 63) + tmp2) * ((uint)*pos)) + (tmp1 << 8);
    tmp2 += 3;
  }

  *nr1 = tmp1;
  *nr2 = tmp2;
}
}  // extern "C"


// Source: ctype-bin.cc
// Lines 278-298
