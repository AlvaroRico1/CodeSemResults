static void safe_put_field(const char *pos, ulong length) {
  if (!pos)
    tee_fputs("NULL", PAGER);
  else {
    int flags =
        MY_PRINT_MB | (opt_raw_data ? 0 : (MY_PRINT_ESC_0 | MY_PRINT_CTRL));
    /* Can't use tee_fputs(), it stops with NUL characters. */
    tee_write(PAGER, pos, length, flags);
  }


// Source: mysql.cc
// Lines 3927-3935
