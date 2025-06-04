static int my_strnncoll_utf32_bin(const CHARSET_INFO *cs, const uchar *s,
                                  size_t slen, const uchar *t, size_t tlen,
                                  bool t_is_prefix) {
  my_wc_t s_wc = 0, t_wc = 0;
  const uchar *se = s + slen;
  const uchar *te = t + tlen;

  while (s < se && t < te) {
    int s_res = my_utf32_uni(cs, &s_wc, s, se);
    int t_res = my_utf32_uni(cs, &t_wc, t, te);

    if (s_res <= 0 || t_res <= 0) {
      /* Incorrect string, compare by char value */
      return my_bincmp(s, se, t, te);
    }
    if (s_wc != t_wc) {
      return s_wc > t_wc ? 1 : -1;
    }

    s += s_res;
    t += t_res;
  }
  return (int)(t_is_prefix ? (t - te) : ((se - s) - (te - t)));
}


// Source: ctype-ucs2.cc
// Lines 2191-2214
