static void hex2octet(uint8 *to, const char *str, uint len) {
  const char *str_end = str + len;
  while (str < str_end) {
    char tmp = char_val(*str++);
    *to++ = (tmp << 4) | char_val(*str++);
  }
}


// Source: password.cc
// Lines 158-164
