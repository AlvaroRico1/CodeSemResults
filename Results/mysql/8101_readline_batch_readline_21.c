char *batch_readline(LINE_BUFFER *line_buff, bool binary_mode) {
  char *pos;
  ulong out_length;

  if (!(pos = intern_read_line(line_buff, &out_length))) return nullptr;
  if (out_length && pos[out_length - 1] == '\n') {
#if defined(_WIN32)
    /*
      On Windows platforms we also need to remove '\r',
      unconditionally.
     */

    /* Remove '\n' */
    if (--out_length && pos[out_length - 1] == '\r') /* Remove '\r' */
      out_length--;
#else
    /*
      On Unix-like platforms we only remove it if we are not
      on binary mode.
     */

    /* Remove '\n' */
    if (--out_length && !binary_mode && pos[out_length - 1] == '\r')
      /* Remove '\r' */
      out_length--;
#endif
  }
  line_buff->read_length = out_length;
  pos[out_length] = 0;
  DBUG_DUMP("Query: ", (unsigned char *)pos, out_length);
  return pos;
}


// Source: readline.cc
// Lines 68-99
