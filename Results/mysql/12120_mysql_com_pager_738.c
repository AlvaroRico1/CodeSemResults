static int com_pager(String *buffer MY_ATTRIBUTE((unused)),
                     char *line MY_ATTRIBUTE((unused))) {
  char pager_name[FN_REFLEN], *end, *param;

  if (status.batch) return 0;
  /* Skip spaces in front of the pager command */
  while (my_isspace(charset_info, *line)) line++;
  /* Skip the pager command */
  param = strchr(line, ' ');
  /* Skip the spaces between the command and the argument */
  while (param && my_isspace(charset_info, *param)) param++;
  if (!param || !strlen(param))  // if pager was not given, use the default
  {
    if (!default_pager_set) {
      tee_fprintf(stdout, "Default pager wasn't set, using stdout.\n");
      opt_nopager = true;
      my_stpcpy(pager, "stdout");
      PAGER = stdout;
      return 0;
    }
    my_stpcpy(pager, default_pager);
  } else {
    end = strmake(pager_name, param, sizeof(pager_name) - 1);
    while (end > pager_name && (my_isspace(charset_info, end[-1]) ||
                                my_iscntrl(charset_info, end[-1])))
      end--;
    end[0] = 0;
    my_stpcpy(pager, pager_name);
    my_stpcpy(default_pager, pager_name);
  }
  opt_nopager = false;
  tee_fprintf(stdout, "PAGER set to '%s'\n", pager);
  return 0;
}


// Source: mysql.cc
// Lines 4014-4047
