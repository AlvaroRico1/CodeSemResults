static int plugin_initialize(st_plugin_int *plugin) {
  int ret = 1;
  DBUG_TRACE;

  mysql_mutex_assert_owner(&LOCK_plugin);
  uint state = plugin->state;
  assert(state == PLUGIN_IS_UNINITIALIZED);

  mysql_mutex_unlock(&LOCK_plugin);

  DEBUG_SYNC(current_thd, "in_plugin_initialize");

  if (plugin_type_initialize[plugin->plugin->type]) {
    if ((*plugin_type_initialize[plugin->plugin->type])(plugin)) {
      LogErr(ERROR_LEVEL, ER_PLUGIN_REGISTRATION_FAILED, plugin->name.str,
             plugin_type_names[plugin->plugin->type].str);
      goto err;
    }

    /* FIXME: Need better solution to transfer the callback function
    array to memcached */
    if (strcmp(plugin->name.str, "InnoDB") == 0) {
      innodb_callback_data = ((handlerton *)plugin->data)->data;
    }
  } else if (plugin->plugin->init) {
    if (strcmp(plugin->name.str, "daemon_memcached") == 0) {
      plugin->data = innodb_callback_data;
    }

    if (plugin->plugin->init(plugin)) {
      LogErr(ERROR_LEVEL, ER_PLUGIN_INIT_FAILED, plugin->name.str);
      goto err;
    }
  }
  state = PLUGIN_IS_READY;  // plugin->init() succeeded

  if (plugin->plugin->status_vars) {
    if (add_status_vars(plugin->plugin->status_vars)) goto err;
  }

  /*
    set the plugin attribute of plugin's sys vars so they are pointing
    to the active plugin
  */
  if (plugin->system_vars) {
    sys_var_pluginvar *var = plugin->system_vars->cast_pluginvar();
    for (;;) {
      var->plugin = plugin;
      if (!var->next) break;
      var = var->next->cast_pluginvar();
    }
  }


// Source: sql_plugin.cc
// Lines 1275-1326
