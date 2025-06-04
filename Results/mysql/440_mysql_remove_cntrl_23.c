static void remove_cntrl(String *buffer) {
  const char *start = buffer->ptr();
  const char *end = start + buffer->length();
  while (start < end && !my_isgraph(charset_info, end[-1])) end--;
  buffer->length((uint)(end - start));
}


// Source: mysql.cc
// Lines 4925-4930
