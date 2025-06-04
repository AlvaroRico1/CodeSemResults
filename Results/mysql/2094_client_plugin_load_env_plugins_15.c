static void load_env_plugins(MYSQL *mysql) {
  char *plugs, *free_env, *s = getenv("LIBMYSQL_PLUGINS");
  char *enable_cleartext_plugin = getenv("LIBMYSQL_ENABLE_CLEARTEXT_PLUGIN");

  if (enable_cleartext_plugin && strchr("1Yy", enable_cleartext_plugin[0]))
    libmysql_cleartext_plugin_enabled = true;

  /* no plugins to load */
  if (!s) return;

  free_env = plugs = my_strdup(key_memory_load_env_plugins, s, MYF(MY_WME));

  do {
    s = strchr(plugs, ';');
    if (s != nullptr) *s = '\0';
    mysql_load_plugin(mysql, plugs, -1, 0);
    if (s != nullptr) plugs = s + 1;
  } while (s != nullptr);

  my_free(free_env);
}


// Source: client_plugin.cc
// Lines 287-307
