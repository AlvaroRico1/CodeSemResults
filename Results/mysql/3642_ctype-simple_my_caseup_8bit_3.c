size_t my_caseup_8bit(const CHARSET_INFO *cs, char *src, size_t srclen,
                      char *dst MY_ATTRIBUTE((unused)),
                      size_t dstlen MY_ATTRIBUTE((unused))) {
  char *end = src + srclen;
  const uchar *map = cs->to_upper;
  assert(src == dst && srclen == dstlen);
  for (; src != end; src++) *src = (char)map[(uchar)*src];
  return srclen;
}


// Source: ctype-simple.cc
// Lines 215-223
