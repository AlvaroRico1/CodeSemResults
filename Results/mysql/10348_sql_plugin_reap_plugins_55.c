static void reap_plugins(void) {
  st_plugin_int *plugin, **reap, **list;

  mysql_mutex_assert_owner(&LOCK_plugin);

  if (!reap_needed) return;

  reap_needed = false;
  const size_t count = plugin_array->size();
  reap = (st_plugin_int **)my_alloca(sizeof(plugin) * (count + 1));
  *(reap++) = nullptr;

  for (size_t idx = 0; idx < count; idx++) {
    plugin = plugin_array->at(idx);
    if (plugin->state == PLUGIN_IS_DELETED && !plugin->ref_count) {
      /* change the status flag to prevent reaping by another thread */
      plugin->state = PLUGIN_IS_DYING;
      *(reap++) = plugin;
    }
  }

  mysql_mutex_unlock(&LOCK_plugin);

  list = reap;
  while ((plugin = *(--list))) {
    if (!opt_initialize)
      LogErr(INFORMATION_LEVEL, ER_PLUGIN_SHUTTING_DOWN_PLUGIN,
             plugin->name.str);
    plugin_deinitialize(plugin, true);
  }

  mysql_mutex_lock(&LOCK_plugin_delete);
  mysql_mutex_lock(&LOCK_plugin);

  while ((plugin = *(--reap))) plugin_del(plugin);

  mysql_mutex_unlock(&LOCK_plugin_delete);
}


// Source: sql_plugin.cc
// Lines 1163-1200
