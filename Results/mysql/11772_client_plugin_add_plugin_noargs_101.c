static struct st_mysql_client_plugin *add_plugin_noargs(
    MYSQL *mysql, struct st_mysql_client_plugin *plugin, void *dlhandle,
    int argc, ...) {
  struct st_mysql_client_plugin *retval = nullptr;
  va_list ap;
  va_start(ap, argc);
  retval = do_add_plugin(mysql, plugin, dlhandle, argc, ap);
  va_end(ap);
  return retval;
}


// Source: client_plugin.cc
// Lines 255-264
