static void log_filter_append(char *out_buf, size_t out_siz, const char *str) {
  size_t out_used = log_bs->length(out_buf);
  size_t out_left = out_siz - out_used;
  char *out_writepos = out_buf + out_used;
  size_t out_needed = log_bs->substitute(out_writepos, out_left, "%s", str);

  if (out_needed >= out_left)    /* buffer exhausted. '\0' terminate */
    out_buf[out_siz - 1] = '\0'; /* purecov: inspected */
}


// Source: log_filter_dragnet.cc
// Lines 354-362
