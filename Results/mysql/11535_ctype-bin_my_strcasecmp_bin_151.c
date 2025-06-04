static int my_strcasecmp_bin(const CHARSET_INFO *cs MY_ATTRIBUTE((unused)),
                             const char *s, const char *t) {
  return strcmp(s, t);
}
}  // extern "C"


// Source: ctype-bin.cc
// Lines 220-224
