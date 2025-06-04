void tee_write(FILE *file, const char *s, size_t slen, int flags) {
#ifdef _WIN32
  bool is_console = my_win_is_console_cached(file);
#endif
  const char *se;
  for (se = s + slen; s < se; s++) {
    const char *t;

    if (flags & MY_PRINT_MB) {
      int mblen;
      if (use_mb(charset_info) && (mblen = my_ismbchar(charset_info, s, se))) {
#ifdef _WIN32
        if (is_console)
          my_win_console_write(charset_info, s, mblen);
        else
#endif
            if (fwrite(s, 1, mblen, file) != (size_t)mblen) {
          perror("fwrite");
        }
        if (opt_outfile) {
          if (fwrite(s, 1, mblen, OUTFILE) != (size_t)mblen) {
            perror("fwrite");
          }
        }
        s += mblen - 1;
        continue;
      }
    }


// Source: mysql.cc
// Lines 4942-4969
