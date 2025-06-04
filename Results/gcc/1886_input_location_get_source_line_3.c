location_get_source_line (const char *file_path, int line)
{
  char *buffer = NULL;
  ssize_t len;

  if (line == 0)
    return char_span (NULL, 0);

  fcache *c = lookup_or_add_file_to_cache_tab (file_path);
  if (c == NULL)
    return char_span (NULL, 0);

  bool read = read_line_num (c, line, &buffer, &len);
  if (!read)
    return char_span (NULL, 0);

  return char_span (buffer, len);
}


// Source: input.c
// Lines 751-768
