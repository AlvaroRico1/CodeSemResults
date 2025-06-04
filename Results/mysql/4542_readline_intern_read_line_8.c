char *intern_read_line(LINE_BUFFER *buffer, ulong *out_length) {
  char *pos;
  size_t length;
  DBUG_TRACE;

  buffer->start_of_line = buffer->end_of_line;
  for (;;) {
    pos = buffer->end_of_line;
    while (*pos != '\n' && pos != buffer->end) pos++;
    if (pos == buffer->end) {
      /*
        fill_buffer() can return NULL on EOF (in which case we abort),
        on error, or when the internal buffer has hit the size limit.
        In the latter case return what we have read so far and signal
        string truncation.
      */
      if (!(length = fill_buffer(buffer))) {
        if (buffer->eof) return nullptr;
      } else if (length == (size_t)-1)
        return nullptr;
      else
        continue;
      pos--; /* break line here */
      buffer->truncated = true;
    } else
      buffer->truncated = false;
    buffer->end_of_line = pos + 1;
    *out_length = (ulong)(pos + 1 - buffer->eof - buffer->start_of_line);

    DBUG_DUMP("Query: ", (unsigned char *)buffer->start_of_line, *out_length);
    return buffer->start_of_line;
  }
}


// Source: readline.cc
// Lines 227-259
