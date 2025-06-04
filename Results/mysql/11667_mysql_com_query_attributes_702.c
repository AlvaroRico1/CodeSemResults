static int com_query_attributes(String *buffer MY_ATTRIBUTE((unused)),
                                char *line) {
  char buff[1024], *param, name[1024];
  memset(buff, 0, sizeof(buff));
  strmake(buff, line, sizeof(buff) - 1);
  param = buff;
  global_attrs->clear(connected ? &mysql : nullptr);
  do {
    param = get_arg(param, param != buff);
    if (!param || !*param) break;

    strncpy(name, param, sizeof(name) - 1);
    param = get_arg(param, true);
    if (!param || !*param) {
      return put_info("Usage: query_attributes name1 value1 name2 value2 ...",
                      INFO_ERROR, 0);
    }

    if (global_attrs->push_param(name, param))
      return put_info("Failed to push a parameter", INFO_ERROR, 0);
  } while (param != 0);
  return 0;
}


// Source: mysql.cc
// Lines 4403-4425
