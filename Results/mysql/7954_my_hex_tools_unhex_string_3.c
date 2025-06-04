unsigned long unhex_string(const char *first, const char *last, char *output) {
  const std::ptrdiff_t length = last - first;
  char *output0 = output;

  if (length % 2) {
    int hex_char = lookup_unhex_low[static_cast<unsigned char>(*first++)];
    if (hex_char > 255) return 0;
    *output++ = static_cast<char>(hex_char);
  }

  for (const char *p = first; p != last; p += 2) {
    int hex_char = lookup_unhex_high[static_cast<unsigned char>(p[0])] |
                   lookup_unhex_low[static_cast<unsigned char>(p[1])];
    if (hex_char > 255) return 0;
    *output++ = static_cast<char>(hex_char);
  }

  return (unsigned long)(output - output0);
}


// Source: my_hex_tools.cc
// Lines 91-109
