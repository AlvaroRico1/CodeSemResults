static int my_strnncoll_utf8(const CHARSET_INFO *cs, const uchar *s,
                             size_t slen, const uchar *t, size_t tlen,
                             bool t_is_prefix) {
  int s_res, t_res;
  my_wc_t s_wc = 0, t_wc = 0;
  const uchar *se = s + slen;
  const uchar *te = t + tlen;
  const MY_UNICASE_INFO *uni_plane = cs->caseinfo;

  while (s < se && t < te) {
    s_res = my_mb_wc_utf8(&s_wc, s, se);
    t_res = my_mb_wc_utf8(&t_wc, t, te);

    if (s_res <= 0 || t_res <= 0) {
      /* Incorrect string, compare byte by byte value */
      return bincmp(s, se, t, te);
    }

    my_tosort_unicode(uni_plane, &s_wc, cs->state);
    my_tosort_unicode(uni_plane, &t_wc, cs->state);

    if (s_wc != t_wc) {
      return s_wc > t_wc ? 1 : -1;
    }

    s += s_res;
    t += t_res;
  }
  return (int)(t_is_prefix ? t - te : ((se - s) - (te - t)));
}


// Source: ctype-utf8.cc
// Lines 5468-5497
