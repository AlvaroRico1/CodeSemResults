void mysql_client_plugin_deinit() {
  int i;
  struct st_client_plugin_int *p;

  if (!initialized) return;

  for (i = 0; i < MYSQL_CLIENT_MAX_PLUGINS; i++)
    for (p = plugin_list[i]; p; p = p->next) {
      if (p->plugin->deinit) p->plugin->deinit();
      if (p->dlhandle) dlclose(p->dlhandle);
    }

  memset(&plugin_list, 0, sizeof(plugin_list));
  initialized = false;
  free_root(&mem_root, MYF(0));
  mysql_mutex_destroy(&LOCK_load_client_plugin);
}


// Source: client_plugin.cc
// Lines 359-375
