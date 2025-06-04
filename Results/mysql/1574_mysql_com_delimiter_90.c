static int com_delimiter(String *buffer MY_ATTRIBUTE((unused)), char *line) {
  char buff[256], *tmp;

  strmake(buff, line, sizeof(buff) - 1);
  tmp = get_arg(buff, false);

  if (!tmp || !*tmp) {
    put_info("DELIMITER must be followed by a 'delimiter' character or string",
             INFO_ERROR);
    return 0;
  } else {
    if (strstr(tmp, "\\")) {
      put_info("DELIMITER cannot contain a backslash character", INFO_ERROR);
      return 0;
    }
  }
  strmake(delimiter, tmp, sizeof(delimiter) - 1);
  delimiter_length = (int)strlen(delimiter);
  delimiter_str = delimiter;
  return 0;
}


// Source: mysql.cc
// Lines 4239-4259
