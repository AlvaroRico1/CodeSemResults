static int fill_uint16(uint16 *a, uint size, const char *str, size_t len) {
  uint i = 0;

  const char *s, *b, *e = str + len;
  for (s = str; s < e; i++) {
    for (; (s < e) && strchr(" \t\r\n", s[0]); s++)
      ;
    b = s;
    for (; (s < e) && !strchr(" \t\r\n", s[0]); s++)
      ;
    if (s == b || i > size) break;
    a[i] = (uint16)strtol(b, nullptr, 16);
  }
  return 0;
}


// Source: ctype.cc
// Lines 348-362
