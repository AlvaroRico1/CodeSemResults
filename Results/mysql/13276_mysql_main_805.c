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


// Source: mysql.cc
// Lines 1241-1265
