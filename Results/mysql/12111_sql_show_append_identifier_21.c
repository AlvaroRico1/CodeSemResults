void append_identifier(const THD *thd, String *packet, const char *name,
                       size_t length, const CHARSET_INFO *from_cs,
                       const CHARSET_INFO *to_cs) {
  const char *name_end;
  char quote_char;
  int q;

  const CHARSET_INFO *cs_info = system_charset_info;
  const char *to_name = name;
  size_t to_length = length;
  String to_string(name, length, from_cs);

  if (from_cs != nullptr && to_cs != nullptr && from_cs != to_cs) {
    uint dummy_errors;
    StringBuffer<MAX_FIELD_WIDTH> convert_buffer;
    convert_buffer.copy(to_string.ptr(), to_string.length(), from_cs, to_cs,
                        &dummy_errors);
    to_string.copy(convert_buffer);
  }

  if (to_cs != nullptr) {
    to_name = to_string.c_ptr();
    to_length = to_string.length();
    cs_info = to_cs;
  }

  q = thd != nullptr ? get_quote_char_for_identifier(thd, to_name, to_length)
                     : '`';

  if (q == EOF) {
    packet->append(to_name, to_length, packet->charset());
    return;
  }

  /*
    The identifier must be quoted as it includes a quote character or
   it's a keyword
  */

  (void)packet->reserve(to_length * 2 + 2);
  quote_char = (char)q;
  packet->append(&quote_char, 1, system_charset_info);

  for (name_end = to_name + to_length; to_name < name_end;
       to_name += to_length) {
    uchar chr = static_cast<uchar>(*to_name);
    to_length = my_mbcharlen(cs_info, chr);
    /*
      my_mbcharlen can return 0 on a wrong multibyte
      sequence. It is possible when upgrading from 4.0,
      and identifier contains some accented characters.
      The manual says it does not work. So we'll just
      change length to 1 not to hang in the endless loop.
    */
    if (!to_length) to_length = 1;
    if (to_length == 1 && chr == static_cast<uchar>(quote_char))
      packet->append(&quote_char, 1, system_charset_info);
    packet->append(to_name, to_length, system_charset_info);
  }
  packet->append(&quote_char, 1, system_charset_info);
}


// Source: sql_show.cc
// Lines 1495-1555
