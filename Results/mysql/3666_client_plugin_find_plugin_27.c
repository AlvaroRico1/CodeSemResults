static struct st_mysql_client_plugin *find_plugin(const char *name, int type) {
  struct st_client_plugin_int *p;

  assert(initialized);
  assert(type >= 0 && type < MYSQL_CLIENT_MAX_PLUGINS);
  if (type < 0 || type >= MYSQL_CLIENT_MAX_PLUGINS) return nullptr;

  for (p = plugin_list[type]; p; p = p->next) {
    if (strcmp(p->plugin->name, name) == 0) return p->plugin;
  }
  return nullptr;
}


// Source: client_plugin.cc
// Lines 150-161
