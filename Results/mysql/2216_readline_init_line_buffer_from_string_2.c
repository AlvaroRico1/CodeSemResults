static bool init_line_buffer_from_string(LINE_BUFFER *buffer, char *str) {
  uint old_length = (uint)(buffer->end - buffer->buffer);
  uint length = (uint)strlen(str);
  if (!(buffer->buffer = buffer->start_of_line = buffer->end_of_line =
            (char *)my_realloc(PSI_NOT_INSTRUMENTED, (uchar *)buffer->buffer,
                               old_length + length + 2,
                               MYF(MY_FAE | MY_ALLOW_ZERO_PTR))))
    return true;
  buffer->end = buffer->buffer + old_length;
  if (old_length) buffer->end[-1] = ' ';
  memcpy(buffer->end, str, length);
  buffer->end[length] = '\n';
  buffer->end[length + 1] = 0;
  buffer->end += length + 1;
  buffer->eof = 1;
  buffer->max_size = 1;
  return false;
}


// Source: readline.cc
// Lines 143-160
