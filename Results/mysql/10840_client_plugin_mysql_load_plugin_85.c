struct st_mysql_client_plugin *mysql_load_plugin(MYSQL *mysql, const char *name,
                                                 int type, int argc, ...) {
  struct st_mysql_client_plugin *p;
  va_list args;
  va_start(args, argc);
  p = mysql_load_plugin_v(mysql, name, type, argc, args);
  va_end(args);
  return p;
}


// Source: client_plugin.cc
// Lines 542-550
