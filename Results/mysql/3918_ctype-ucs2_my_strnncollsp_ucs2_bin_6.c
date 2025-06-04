static int my_strnncollsp_ucs2_bin(
    const CHARSET_INFO *cs MY_ATTRIBUTE((unused)), const uchar *s, size_t slen,
    const uchar *t, size_t tlen) {
  const uchar *se, *te;
  size_t minlen;

  /* extra safety to make sure the lengths are even numbers */
  slen = (slen >> 1) << 1;
  tlen = (tlen >> 1) << 1;

  se = s + slen;
  te = t + tlen;

  for (minlen = std::min(slen, tlen); minlen; minlen -= 2) {
    int s_wc = s[0] * 256 + s[1];
    int t_wc = t[0] * 256 + t[1];
    if (s_wc != t_wc) return s_wc > t_wc ? 1 : -1;

    s += 2;
    t += 2;
  }

  if (slen != tlen) {
    int swap = 1;
    if (slen < tlen) {
      s = t;
      se = te;
      swap = -1;
    }

    for (; s < se; s += 2) {
      if (s[0] || s[1] != ' ') return (s[0] == 0 && s[1] < ' ') ? -swap : swap;
    }
  }
  return 0;
}

static void my_hash_sort_ucs2_bin(const CHARSET_INFO *cs MY_ATTRIBUTE((unused)),
                                  const uchar *key, size_t len, uint64 *nr1,
                                  uint64 *nr2) {
  const uchar *pos = key;
  uint64 tmp1;
  uint64 tmp2;

  key += len;

  while (key > pos + 1 && key[-1] == ' ' && key[-2] == '\0') key -= 2;

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


// Source: ctype-ucs2.cc
// Lines 2754-2813
