static void my_hash_sort_latin1_de(
    const CHARSET_INFO *cs MY_ATTRIBUTE((unused)), const uchar *key, size_t len,
    uint64 *nr1, uint64 *nr2) {
  const uchar *end;
  uint64 tmp1;
  uint64 tmp2;

  /*
    Remove end space. We have to do this to be able to compare
    'AE' and 'Ä' as identical
  */
  end = skip_trailing_space(key, len);

  tmp1 = *nr1;
  tmp2 = *nr2;

  for (; key < end; key++) {
    uint X = (uint)combo1map[(uint)*key];
    tmp1 ^= (uint64)((((uint)tmp1 & 63) + tmp2) * X) + (tmp1 << 8);
    tmp2 += 3;
    if ((X = combo2map[*key])) {
      tmp1 ^= (uint64)((((uint)tmp1 & 63) + tmp2) * X) + (tmp1 << 8);
      tmp2 += 3;
    }
  }

  *nr1 = tmp1;
  *nr2 = tmp2;
}
}  // extern "C"


// Source: ctype-latin1.cc
// Lines 590-619
