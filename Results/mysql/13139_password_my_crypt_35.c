static void my_crypt(char *to, const uchar *s1, const uchar *s2, uint len) {
  const uint8 *s1_end = s1 + len;
  while (s1 < s1_end) *to++ = *s1++ ^ *s2++;
}


// Source: password.cc
// Lines 178-181
