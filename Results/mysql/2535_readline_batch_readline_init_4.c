LINE_BUFFER *batch_readline_init(ulong max_size, FILE *file) {
  LINE_BUFFER *line_buff;

#ifndef _WIN32
  MY_STAT input_file_stat;
  if (my_fstat(fileno(file), &input_file_stat) ||
      MY_S_ISDIR(input_file_stat.st_mode) ||
      MY_S_ISBLK(input_file_stat.st_mode))
    return nullptr;
#endif

  if (!(line_buff =
            (LINE_BUFFER *)my_malloc(PSI_NOT_INSTRUMENTED, sizeof(*line_buff),
                                     MYF(MY_WME | MY_ZEROFILL))))
    return nullptr;
  if (init_line_buffer(line_buff, my_fileno(file), batch_io_size, max_size)) {
    my_free(line_buff);
    return nullptr;
  }
  return line_buff;
}


// Source: readline.cc
// Lines 46-66
