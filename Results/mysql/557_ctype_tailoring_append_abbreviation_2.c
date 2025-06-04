static int tailoring_append_abbreviation(MY_XML_PARSER *st, const char *fmt,
                                         size_t len, const char *attr) {
  size_t clen;
  const char *attrend = attr + len;
  my_wc_t wc;

  for (; (clen = scan_one_character(attr, attrend, &wc)) > 0; attr += clen) {
    assert(attr < attrend);
    if (tailoring_append(st, fmt, clen, attr) != MY_XML_OK) return MY_XML_ERROR;
  }
  return MY_XML_OK;
}


// Source: ctype.cc
// Lines 415-426
