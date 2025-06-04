static int com_server_help(String *buffer MY_ATTRIBUTE((unused)),
                           char *line MY_ATTRIBUTE((unused)), char *help_arg) {
  MYSQL_ROW cur;
  const char *server_cmd;
  char cmd_buf[100 + 1];
  MYSQL_RES *result;
  int error;

  if (help_arg[0] != '\'') {
    char *end_arg = strend(help_arg);
    if (--end_arg) {
      while (my_isspace(charset_info, *end_arg)) end_arg--;
      *++end_arg = '\0';
    }
    (void)strxnmov(cmd_buf, sizeof(cmd_buf), "help '", help_arg, "'", NullS);
  } else


// Source: mysql.cc
// Lines 3118-3133
