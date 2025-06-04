ulong STDCALL mysql_get_server_version(MYSQL *mysql) {
  ulong major = 0, minor = 0, version = 0;

  if (mysql->server_version) {
    char *pos = mysql->server_version, *end_pos;
    major = strtoul(pos, &end_pos, 10);
    pos = end_pos + 1;
    minor = strtoul(pos, &end_pos, 10);
    pos = end_pos + 1;
    version = strtoul(pos, &end_pos, 10);
  } else {
    set_mysql_error(mysql, CR_COMMANDS_OUT_OF_SYNC, unknown_sqlstate);
  }


// Source: client.cc
// Lines 8574-8586
