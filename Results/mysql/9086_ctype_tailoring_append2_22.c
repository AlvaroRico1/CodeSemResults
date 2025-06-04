static int tailoring_append2(MY_XML_PARSER *st, const char *fmt, size_t len1,
                             const char *attr1, size_t len2,
                             const char *attr2) {
  struct my_cs_file_info *i = (struct my_cs_file_info *)st->user_data;
  size_t newlen = i->tailoring_length + len1 + len2 + 64; /* 64 for format */
  if (MY_XML_OK == my_charset_file_tailoring_realloc(i, newlen)) {
    char *dst = i->tailoring + i->tailoring_length;
    sprintf(dst, fmt, (int)len1, attr1, (int)len2, attr2);
    i->tailoring_length += strlen(dst);
    return MY_XML_OK;
  }
  return MY_XML_ERROR;
}


// Source: ctype.cc
// Lines 377-389
