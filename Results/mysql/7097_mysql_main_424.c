int main(int argc, char *argv[]) {
  char buff[80];

  MY_INIT(argv[0]);
  DBUG_TRACE;
  DBUG_PROCESS(argv[0]);

  charset_index = get_command_index('C');
  delimiter_index = get_command_index('d');
  delimiter_str = delimiter;
  default_prompt = my_strdup(
      PSI_NOT_INSTRUMENTED,
      getenv("MYSQL_PS1") ? getenv("MYSQL_PS1") : "mysql> ", MYF(MY_WME));
  current_prompt = my_strdup(PSI_NOT_INSTRUMENTED, default_prompt, MYF(MY_WME));
  prompt_counter = 0;

  outfile[0] = 0;              // no (default) outfile
  my_stpcpy(pager, "stdout");  // the default, if --pager wasn't given
  {
    char *tmp = getenv("PAGER");
    if (tmp && strlen(tmp)) {
      default_pager_set = true;
      my_stpcpy(default_pager, tmp);
    }
  }
  if (!isatty(0) || !isatty(1)) {
    status.batch = true;
    opt_silent = 1;
    ignore_errors = false;
  } else
    status.add_to_history = true;
  status.exit_status = 1;

  {
    /*
     The file descriptor-layer may be out-of-sync with the file-number layer,
     so we make sure that "stdout" is really open.  If its file is closed then
     explicitly close the FD layer.
    */
    int stdout_fileno_copy;
    stdout_fileno_copy = dup(fileno(stdout)); /* Okay if fileno fails. */
    if (stdout_fileno_copy == -1) {
      fclose(stdout);
#ifdef LINUX_ALPINE
      // On Alpine linux we need to open a dummy file, so that the first
      // call to socket() does not get file number 1
      // If socket gets file number 1, then everything printed to stdout
      // will be sent back to the server over the socket connection.
      fopen("/dev/null", "r");
#endif
    } else
      close(stdout_fileno_copy); /* Clean up dup(). */
  }


// Source: mysql.cc
// Lines 1241-1293
