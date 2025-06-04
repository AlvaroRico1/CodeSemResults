char *octet2hex(char *to, const char *str, uint len) {
  const char *str_end = str + len;
  for (; str != str_end; ++str) {
    *to++ = _dig_vec_upper[((uchar)*str) >> 4];
    *to++ = _dig_vec_upper[((uchar)*str) & 0x0F];
  }
  *to = '\0';
  return to;
}


// Source: password.cc
// Lines 138-146
