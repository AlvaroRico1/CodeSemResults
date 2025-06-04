static int my_mb_wc_latin1(const CHARSET_INFO *cs MY_ATTRIBUTE((unused)),
                           my_wc_t *wc, const uchar *str, const uchar *end) {
  if (str >= end) return MY_CS_TOOSMALL;

  *wc = cs_to_uni[*str];
  return (!wc[0] && str[0]) ? -1 : 1;
}

static int my_wc_mb_latin1(const CHARSET_INFO *cs MY_ATTRIBUTE((unused)),
                           my_wc_t wc, uchar *str, uchar *end) {
  const uchar *pl;

  if (str >= end) return MY_CS_TOOSMALL;

  if (wc > 0xFFFF) return MY_CS_ILUNI;

  pl = uni_to_cs[wc >> 8];
  str[0] = pl ? pl[wc & 0xFF] : '\0';
  return (!str[0] && wc) ? MY_CS_ILUNI : 1;
}
}  // extern "C"


// Source: ctype-latin1.cc
// Lines 317-337
