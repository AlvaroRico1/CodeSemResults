static int cs_enter(MY_XML_PARSER *st, const char *attr, size_t len) {
  struct my_cs_file_info *i = (struct my_cs_file_info *)st->user_data;
  struct my_cs_file_section_st *s = cs_file_sec(attr, len);
  int state = s ? s->state : 0;

  switch (state) {
    case 0:
      i->loader->reporter(WARNING_LEVEL, EE_UNKNOWN_LDML_TAG, (int)len, attr);
      break;

    case _CS_CHARSET:
      my_charset_file_reset_charset(i);
      break;

    case _CS_COLLATION:
      my_charset_file_reset_collation(i);
      break;

    case _CS_RESET:
      return tailoring_append(st, " &", 0, nullptr);

    default:
      break;
  }
  return MY_XML_OK;
}


// Source: ctype.cc
// Lines 429-454
