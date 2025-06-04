static size_t my_well_formed_len_ucs2(
    const CHARSET_INFO *cs MY_ATTRIBUTE((unused)), const char *b, const char *e,
    size_t nchars, int *error) {
  /* Ensure string length is dividable with 2 */
  size_t nbytes = ((size_t)(e - b)) & ~(size_t)1;
  *error = 0;
  nchars *= 2;
  return std::min(nbytes, nchars);
}


// Source: ctype-ucs2.cc
// Lines 2701-2709
