ulong STDCALL mysql_hex_string(char *to, const char *from, ulong length) {
  char *to0 = to;
  const char *end;

  for (end = from + length; from < end; from++) {
    *to++ = _dig_vec_upper[((unsigned char)*from) >> 4];
    *to++ = _dig_vec_upper[((unsigned char)*from) & 0x0F];
  }
  *to = '\0';
  return (ulong)(to - to0);
}


// Source: libmysql.cc
// Lines 1032-1042
