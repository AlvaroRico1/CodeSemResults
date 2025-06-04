size_t my_lengthsp_8bit(const CHARSET_INFO *cs MY_ATTRIBUTE((unused)),
                        const char *ptr, size_t length) {
  const char *end;
  end = (const char *)skip_trailing_space((const uchar *)ptr, length);
  return (size_t)(end - ptr);
}


// Source: ctype-simple.cc
// Lines 927-932
