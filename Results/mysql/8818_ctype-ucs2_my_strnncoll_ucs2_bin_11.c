static int my_strnncoll_ucs2_bin(const CHARSET_INFO *cs, const uchar *s,
                                 size_t slen, const uchar *t, size_t tlen,
                                 bool t_is_prefix) {
  int s_res, t_res;
  my_wc_t s_wc = 0, t_wc = 0;
  const uchar *se = s + slen;
  const uchar *te = t + tlen;

  while (s < se && t < te) {
    s_res = my_ucs2_uni(cs, &s_wc, s, se);
    t_res = my_ucs2_uni(cs, &t_wc, t, te);

    if (s_res <= 0 || t_res <= 0) {
      /* Incorrect string, compare by char value */
      return ((int)s[0] - (int)t[0]);
    }
    if (s_wc != t_wc) {
      return s_wc > t_wc ? 1 : -1;
    }

    s += s_res;
    t += t_res;
  }
  return (int)(t_is_prefix ? t - te : ((se - s) - (te - t)));
}


// Source: ctype-ucs2.cc
// Lines 2728-2752
