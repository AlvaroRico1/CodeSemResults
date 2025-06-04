static int my_strcasecmp_gb18030(const CHARSET_INFO *cs, const char *s,
                                 const char *t) {
  size_t s_length = strlen(s);
  size_t t_length = strlen(t);
  int res = my_strnncoll_gb18030_internal(cs, (const uchar **)&s, s_length,
                                          (const uchar **)&t, t_length);

  return res ? res : (int)(s_length - t_length);
}
}  // extern "C"


// Source: ctype-gb18030.cc
// Lines 20109-20118
