static int sql_connect(char *host, char *database, char *user, char *password,
                       uint silent) {
  bool message = false;
  uint count = 0;
  int error;
  for (;;) {
    if ((error = sql_real_connect(host, database, user, password, wait_flag)) >=
        0) {
      if (count) {
        tee_fputs("\n", stderr);
        (void)fflush(stderr);
      }
      return error;
    }
    if (!wait_flag) return ignore_errors ? -1 : 1;
    if (!message && !silent) {
      message = true;
      tee_fputs("Waiting", stderr);
      (void)fflush(stderr);
    }
    (void)sleep(wait_time);
    if (!silent) {
      putc('.', stderr);
      (void)fflush(stderr);
      count++;
    }
  }
}


// Source: mysql.cc
// Lines 4702-4729
