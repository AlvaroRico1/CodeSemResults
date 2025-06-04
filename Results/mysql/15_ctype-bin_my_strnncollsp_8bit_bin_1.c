static int my_strnncollsp_8bit_bin(
    const CHARSET_INFO *cs MY_ATTRIBUTE((unused)), const uchar *a,
    size_t a_length, const uchar *b, size_t b_length) {
  const uchar *end;
  size_t length;
  int res;

  end = a + (length = std::min(a_length, b_length));
  while (a < end) {
    if (*a++ != *b++) return ((int)a[-1] - (int)b[-1]);
  }
  res = 0;
  if (a_length != b_length) {
    int swap = 1;
    /*
      Check the next not space character of the longer key. If it's < ' ',
      then it's smaller than the other key.
    */
    if (a_length < b_length) {
      /* put shorter key in s */
      a_length = b_length;
      a = b;
      swap = -1; /* swap sign of result */
      res = -res;
    }
    for (end = a + a_length - length; a < end; a++) {
      if (*a != ' ') return (*a < ' ') ? -swap : swap;
    }
  }


// Source: ctype-bin.cc
// Lines 174-202
