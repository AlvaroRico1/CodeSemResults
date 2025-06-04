static void report_errors() {
  unsigned long l;
  const char *file;
  const char *data;
  int line, flags;

  DBUG_TRACE;

  while ((l = ERR_get_error_line_data(&file, &line, &data, &flags)) > 0) {
#ifndef NDEBUG /* Avoid warning */
    char buf[200];
    DBUG_PRINT("error", ("OpenSSL: %s:%s:%d:%s\n", ERR_error_string(l, buf),
                         file, line, (flags & ERR_TXT_STRING) ? data : ""));
#endif
  }
}


// Source: viosslfactories.cc
// Lines 205-220
