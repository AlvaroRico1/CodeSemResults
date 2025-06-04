static int my_strnncoll_8bit_bin(const CHARSET_INFO *cs MY_ATTRIBUTE((unused)),
                                 const uchar *s, size_t slen, const uchar *t,
                                 size_t tlen, bool t_is_prefix) {
  size_t len = std::min(slen, tlen);
  int cmp = memcmp(s, t, len);
  return cmp ? cmp : (int)((t_is_prefix ? len : slen) - tlen);
}


// Source: ctype-bin.cc
// Lines 144-150
