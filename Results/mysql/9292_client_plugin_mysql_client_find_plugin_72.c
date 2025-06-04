struct st_mysql_client_plugin *mysql_client_find_plugin(MYSQL *mysql,
                                                        const char *name,
                                                        int type) {
  struct st_mysql_client_plugin *p;

  DBUG_TRACE;
  DBUG_PRINT("entry", ("name=%s, type=%d", name, type));
  if (is_not_initialized(mysql, name)) return nullptr;

  if (type < 0 || type >= MYSQL_CLIENT_MAX_PLUGINS) {
    set_mysql_extended_error(
        mysql, CR_AUTH_PLUGIN_CANNOT_LOAD, unknown_sqlstate,
        ER_CLIENT(CR_AUTH_PLUGIN_CANNOT_LOAD), name, "invalid type");
  }

  if ((p = find_plugin(name, type))) {
    DBUG_PRINT("leave", ("found %p", p));
    return p;
  }

  /* not found, load it */
  p = mysql_load_plugin(mysql, name, type, 0);
  DBUG_PRINT("leave", ("loaded %p", p));
  return p;
}


// Source: client_plugin.cc
// Lines 553-577
