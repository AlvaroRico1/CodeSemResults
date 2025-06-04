bool check_host_name(const LEX_CSTRING &str) {
  const char *name = str.str;
  const char *end = str.str + str.length;
  if (check_string_byte_length(str, ER_THD(current_thd, ER_HOSTNAME),
                               HOSTNAME_LENGTH))
    return true;

  while (name != end) {
    if (*name == '@') {
      my_printf_error(ER_UNKNOWN_ERROR,
                      "Malformed hostname (illegal symbol: '%c')", MYF(0),
                      *name);
      return true;
    }
    name++;
  }
  return false;
}


// Source: sql_parse.cc
// Lines 6631-6648
