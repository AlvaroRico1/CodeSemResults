void hash_password(ulong *result, const char *password, uint password_len) {
  ulong nr = 1345345333L, add = 7, nr2 = 0x12345671L;
  ulong tmp;
  const char *password_end = password + password_len;
  for (; password < password_end; password++) {
    if (*password == ' ' || *password == '\t')
      continue; /* skip space in password */
    tmp = (ulong)(uchar)*password;
    nr ^= (((nr & 63) + add) * tmp) + (nr << 8);
    nr2 += (nr2 << 8) ^ nr;
    add += tmp;
  }
  result[0] = nr & (((ulong)1L << 31) - 1L); /* Don't use sign bit (str2int) */
  ;
  result[1] = nr2 & (((ulong)1L << 31) - 1L);
}


// Source: password.cc
// Lines 99-114
