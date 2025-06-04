static const char *require_quotes(const char *name, size_t name_length) {
  bool pure_digit = true;
  const char *end = name + name_length;

  for (; name < end; name++) {
    uchar chr = (uchar)*name;
    uint length = my_mbcharlen(system_charset_info, chr);
    if (length == 0 || (length == 1 && !system_charset_info->ident_map[chr]))
      return name;
    if (length == 1 && (chr < '0' || chr > '9')) pure_digit = false;
  }
  if (pure_digit) return name;
  return nullptr;
}


// Source: sql_show.cc
// Lines 1422-1435
