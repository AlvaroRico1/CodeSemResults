unsigned long hex_string(char *to, const char *from, unsigned long length) {
  char *to0 = to;
  const char *end;

  for (end = from + length; from < end; from++) {
    *to++ = _dig_vec_upper[((unsigned char)*from) >> 4];
    *to++ = _dig_vec_upper[((unsigned char)*from) & 0x0F];
  }
  return (unsigned long)(to - to0);
}


// Source: my_hex_tools.cc
// Lines 111-120
