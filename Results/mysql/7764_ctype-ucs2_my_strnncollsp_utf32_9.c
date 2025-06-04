static int my_strnncollsp_utf32(const CHARSET_INFO *cs, const uchar *s,
                                size_t slen, const uchar *t, size_t tlen) {
  int res;
  my_wc_t s_wc = 0, t_wc = 0;
  const uchar *se = s + slen, *te = t + tlen;
  const MY_UNICASE_INFO *uni_plane = cs->caseinfo;

  assert((slen % 4) == 0);
  assert((tlen % 4) == 0);

  while (s < se && t < te) {
    int s_res = my_utf32_uni(cs, &s_wc, s, se);
    int t_res = my_utf32_uni(cs, &t_wc, t, te);

    if (s_res <= 0 || t_res <= 0) {
      /* Incorrect string, compare bytewise */
      return my_bincmp(s, se, t, te);
    }

    my_tosort_utf32(uni_plane, &s_wc);
    my_tosort_utf32(uni_plane, &t_wc);

    if (s_wc != t_wc) {
      return s_wc > t_wc ? 1 : -1;
    }

    s += s_res;
    t += t_res;
  }

  slen = (size_t)(se - s);
  tlen = (size_t)(te - t);
  res = 0;

  if (slen != tlen) {
    int s_res, swap = 1;
    if (slen < tlen) {
      slen = tlen;
      s = t;
      se = te;
      swap = -1;
      res = -res;
    }

    for (; s < se; s += s_res) {
      if ((s_res = my_utf32_uni(cs, &s_wc, s, se)) < 0) {
        assert(0);
        return 0;
      }
      if (s_wc != ' ') return (s_wc < ' ') ? -swap : swap;
    }
  }
  return res;
}


// Source: ctype-ucs2.cc
// Lines 1815-1868
