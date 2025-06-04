static int my_mb_wc_bin(const CHARSET_INFO *cs MY_ATTRIBUTE((unused)),
                        my_wc_t *wc, const uchar *str, const uchar *end) {
  if (str >= end) return MY_CS_TOOSMALL;

  *wc = str[0];
  return 1;
}

static int my_wc_mb_bin(const CHARSET_INFO *cs MY_ATTRIBUTE((unused)),
                        my_wc_t wc, uchar *s, uchar *e) {
  if (s >= e) return MY_CS_TOOSMALL;

  if (wc < 256) {
    s[0] = (char)wc;
    return 1;
  }
  return MY_CS_ILUNI;
}
}  // extern "C"


// Source: ctype-bin.cc
// Lines 232-250
