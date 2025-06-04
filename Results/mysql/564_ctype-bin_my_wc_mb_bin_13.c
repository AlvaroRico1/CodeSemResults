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
// Lines 240-250
