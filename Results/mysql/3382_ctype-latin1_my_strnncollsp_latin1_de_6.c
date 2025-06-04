static int my_strnncollsp_latin1_de(
    const CHARSET_INFO *cs MY_ATTRIBUTE((unused)), const uchar *a,
    size_t a_length, const uchar *b, size_t b_length) {
  const uchar *a_end = a + a_length, *b_end = b + b_length;
  uchar a_char, a_extend = 0, b_char, b_extend = 0;
  int res;

  while ((a < a_end || a_extend) && (b < b_end || b_extend)) {
    if (a_extend) {
      a_char = a_extend;
      a_extend = 0;
    } else {
      a_extend = combo2map[*a];
      a_char = combo1map[*a++];
    }
    if (b_extend) {
      b_char = b_extend;
      b_extend = 0;
    } else {
      b_extend = combo2map[*b];
      b_char = combo1map[*b++];
    }
    if (a_char != b_char) return (int)a_char - (int)b_char;
  }
  /* Check if double character last */
  if (a_extend) return 1;
  if (b_extend) return -1;

  res = 0;
  if (a != a_end || b != b_end) {
    int swap = 1;
    /*
      Check the next not space character of the longer key. If it's < ' ',
      then it's smaller than the other key.
    */
    if (a == a_end) {
      /* put shorter key in a */
      a_end = b_end;
      a = b;
      swap = -1; /* swap sign of result */
      res = -res;
    }
    for (; a < a_end; a++) {
      if (*a != ' ') return (*a < ' ') ? -swap : swap;
    }
  }
  return res;
}


// Source: ctype-latin1.cc
// Lines 524-571
