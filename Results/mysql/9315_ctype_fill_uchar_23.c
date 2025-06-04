static int fill_uchar(uchar *a, uint size, const char *str, size_t len) {
  uint i = 0;
  const char *s, *b, *e = str + len;

  for (s = str; s < e; i++) {
    for (; (s < e) && strchr(" \t\r\n", s[0]); s++)
      ;
    b = s;
    for (; (s < e) && !strchr(" \t\r\n", s[0]); s++)
      ;
    if (s == b || i > size) break;
    a[i] = (uchar)strtoul(b, nullptr, 16);
  }
  return 0;
}


// Source: ctype.cc
// Lines 332-346
