static size_t my_well_formed_len_big5(
    const CHARSET_INFO *cs MY_ATTRIBUTE((unused)), const char *b, const char *e,
    size_t pos, int *error) {
  const char *b0 = b;
  const char *emb = e - 1; /* Last possible end of an MB character */

  *error = 0;
  while (pos-- && b < e) {
    if ((uchar)b[0] < 128) {
      /* Single byte ascii character */
      b++;
    } else if ((b < emb) && isbig5code((uchar)*b, (uchar)b[1])) {
      /* Double byte character */
      b += 2;
    } else {
      /* Wrong byte sequence */
      *error = 1;
      break;
    }
  }
  return (size_t)(b - b0);
}


// Source: ctype-big5.cc
// Lines 6468-6489
