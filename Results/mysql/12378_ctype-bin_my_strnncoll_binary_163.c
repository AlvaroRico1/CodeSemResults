static int my_strnncoll_binary(const CHARSET_INFO *cs MY_ATTRIBUTE((unused)),
                               const uchar *s, size_t slen, const uchar *t,
                               size_t tlen, bool t_is_prefix) {
  size_t len = std::min(slen, tlen);
  const int cmp = len == 0 ? 0 : memcmp(s, t, len);  // memcmp(a, b, 0) == 0
  return cmp ? cmp : (int)((t_is_prefix ? len : slen) - tlen);
}


// Source: ctype-bin.cc
// Lines 101-107
