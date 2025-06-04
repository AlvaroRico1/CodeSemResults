static int my_strnncollsp_utf32_bin(
    const CHARSET_INFO *cs MY_ATTRIBUTE((unused)), const uchar *s, size_t slen,
    const uchar *t, size_t tlen) {
  const uchar *se, *te;
  size_t minlen;

  assert((slen % 4) == 0);
  assert((tlen % 4) == 0);

  se = s + slen;
  te = t + tlen;

  for (minlen = std::min(slen, tlen); minlen; minlen -= 4) {
    my_wc_t s_wc = my_utf32_get(s);
    my_wc_t t_wc = my_utf32_get(t);
    if (s_wc != t_wc) return s_wc > t_wc ? 1 : -1;

    s += 4;
    t += 4;
  }

  if (slen != tlen) {
    int swap = 1;
    if (slen < tlen) {
      s = t;
      se = te;
      swap = -1;
    }

    for (; s < se; s += 4) {
      my_wc_t s_wc = my_utf32_get(s);
      if (s_wc != ' ') return (s_wc < ' ') ? -swap : swap;
    }
  }
  return 0;
}

static size_t my_scan_utf32(const CHARSET_INFO *cs, const char *str,
                            const char *end, int sequence_type) {
  const char *str0 = str;

  switch (sequence_type) {
    case MY_SEQ_SPACES:
      for (; str < end;) {
        my_wc_t wc;
        int res = my_utf32_uni(cs, &wc, pointer_cast<const uchar *>(str),
                               pointer_cast<const uchar *>(end));
        if (res < 0 || wc != ' ') break;
        str += res;
      }
      return (size_t)(str - str0);
    default:
      return 0;
  }
}
}  // extern "C"


// Source: ctype-ucs2.cc
// Lines 2223-2278
