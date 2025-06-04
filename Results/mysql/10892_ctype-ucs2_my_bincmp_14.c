static inline int my_bincmp(const uchar *s, const uchar *se, const uchar *t,
                            const uchar *te) {
  int slen = (int)(se - s), tlen = (int)(te - t);
  int len = std::min(slen, tlen);
  int cmp = memcmp(s, t, len);
  return cmp ? cmp : slen - tlen;
}


// Source: ctype-ucs2.cc
// Lines 63-69
