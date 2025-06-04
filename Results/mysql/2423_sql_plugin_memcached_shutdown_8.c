void memcached_shutdown(void) {
  if (initialized) {
    for (st_plugin_int **it = plugin_array->begin(); it != plugin_array->end();
         ++it) {
      st_plugin_int *plugin = *it;

      if (plugin->state == PLUGIN_IS_READY &&
          strcmp(plugin->name.str, "daemon_memcached") == 0) {
        plugin_deinitialize(plugin, true);

        mysql_mutex_lock(&LOCK_plugin_delete);
        mysql_mutex_lock(&LOCK_plugin);
        plugin->state = PLUGIN_IS_DYING;
        plugin_del(plugin);
        mysql_mutex_unlock(&LOCK_plugin);
        mysql_mutex_unlock(&LOCK_plugin_delete);
      }
    }


// Source: sql_plugin.cc
// Lines 1985-2002
