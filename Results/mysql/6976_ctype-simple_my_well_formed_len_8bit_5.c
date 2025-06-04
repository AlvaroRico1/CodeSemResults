size_t my_well_formed_len_8bit(const CHARSET_INFO *cs MY_ATTRIBUTE((unused)),
                               const char *start, const char *end,
                               size_t nchars, int *error) {
  size_t nbytes = (size_t)(end - start);
  *error = 0;
  return std::min(nbytes, nchars);
}


// Source: ctype-simple.cc
// Lines 919-925
