static int tailoring_append(MY_XML_PARSER *st, const char *fmt, size_t len,
                            const char *attr) {
  struct my_cs_file_info *i = (struct my_cs_file_info *)st->user_data;
  size_t newlen = i->tailoring_length + len + 64; /* 64 for format */
  if (MY_XML_OK == my_charset_file_tailoring_realloc(i, newlen)) {
    char *dst = i->tailoring + i->tailoring_length;
    sprintf(dst, fmt, (int)len, attr);
    i->tailoring_length += strlen(dst);
    return MY_XML_OK;
  }


// Source: ctype.cc
// Lines 364-373
