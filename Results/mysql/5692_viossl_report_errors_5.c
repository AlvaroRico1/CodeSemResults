static void report_errors(SSL *ssl) {
  unsigned long l;
  const char *file;
  const char *data;
  int line, flags = 0;
  char buf[512];

  DBUG_TRACE;

  while ((l = ERR_get_error_line_data(&file, &line, &data, &flags))) {
    DBUG_PRINT("error", ("OpenSSL: %s:%s:%d:%s\n", ERR_error_string(l, buf),
                         file, line, (flags & ERR_TXT_STRING) ? data : ""));
  }

  if (ssl)
    DBUG_PRINT("error",
               ("error: %s", ERR_error_string(SSL_get_error(ssl, l), buf)));

  DBUG_PRINT("info", ("socket_errno: %d", socket_errno));
}


// Source: viossl.cc
// Lines 123-142
