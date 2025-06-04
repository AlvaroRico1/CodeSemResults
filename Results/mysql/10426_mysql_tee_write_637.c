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

    if ((flags & MY_PRINT_XML) && (t = array_value(xmlmeta, *s)))
      tee_fputs(t, file);
    else if ((flags & MY_PRINT_SPS_0) && *s == '\0')
      tee_putc((int)' ', file);  // This makes everything hard
    else if ((flags & MY_PRINT_ESC_0) && *s == '\0')
      tee_fputs("\\0", file);  // This makes everything hard
    else if ((flags & MY_PRINT_CTRL) && *s == '\t')
      tee_fputs("\\t", file);  // This would destroy tab format
    else if ((flags & MY_PRINT_CTRL) && *s == '\n')
      tee_fputs("\\n", file);  // This too
    else if ((flags & MY_PRINT_CTRL) && *s == '\\')
      tee_fputs("\\\\", file);
    else {
#ifdef _WIN32
      if (is_console)
        my_win_console_putc(charset_info, (int)*s);
      else
#endif
        putc((int)*s, file);
      if (opt_outfile) putc((int)*s, OUTFILE);
    }
  }
}


// Source: mysql.cc
// Lines 4942-4993
