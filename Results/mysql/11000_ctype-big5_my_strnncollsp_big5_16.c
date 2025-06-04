static int my_strnncollsp_big5(const CHARSET_INFO *cs MY_ATTRIBUTE((unused)),
                               const uchar *a, size_t a_length, const uchar *b,
                               size_t b_length) {
  size_t length = std::min(a_length, b_length);
  int res = my_strnncoll_big5_internal(&a, &b, length);

  if (!res && a_length != b_length) {
    const uchar *end;
    int swap = 1;
    /*
      Check the next not space character of the longer key. If it's < ' ',
      then it's smaller than the other key.
    */
    if (a_length < b_length) {
      /* put longer key in a */
      a_length = b_length;
      a = b;
      swap = -1; /* swap sign of result */
      res = -res;
    }
    for (end = a + a_length - length; a < end; a++) {
      if (*a != ' ') return (*a < ' ') ? -swap : swap;
    }
  }


// Source: ctype-big5.cc
// Lines 1233-1256
