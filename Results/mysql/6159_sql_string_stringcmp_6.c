int stringcmp(const String *s, const String *t) {
  size_t s_len = s->length();
  size_t t_len = t->length();
  size_t len = min(s_len, t_len);
  int cmp = (len == 0) ? 0 : memcmp(s->ptr(), t->ptr(), len);
  return (cmp) ? cmp : static_cast<int>(s_len) - static_cast<int>(t_len);
}


// Source: sql_string.cc
// Lines 743-749
