bool plugin_foreach_with_mask(THD *thd, plugin_foreach_func **funcs, int type,
                              uint state_mask, void *arg) {
  size_t idx, total;
  st_plugin_int *plugin, **plugins;
  int version = plugin_array_version;
  DBUG_TRACE;

  if (!initialized) return false;

  state_mask = ~state_mask;  // do it only once

  mysql_mutex_lock(&LOCK_plugin);
  total = type == MYSQL_ANY_PLUGIN ? plugin_array->size()
                                   : plugin_hash[type]->size();
  /*
    Do the alloca out here in case we do have a working alloca:
        leaving the nested stack frame invalidates alloca allocation.
  */
  plugins = (st_plugin_int **)my_alloca(total * sizeof(plugin));
  if (type == MYSQL_ANY_PLUGIN) {
    for (idx = 0; idx < total; idx++) {
      plugin = plugin_array->at(idx);
      plugins[idx] = !(plugin->state & state_mask) ? plugin : nullptr;
    }
  } else {
    collation_unordered_map<std::string, st_plugin_int *> *hash =
        plugin_hash[type];
    idx = 0;
    for (const auto &key_and_value : *hash) {
      plugin = key_and_value.second;
      plugins[idx++] = !(plugin->state & state_mask) ? plugin : nullptr;
    }
  }
  mysql_mutex_unlock(&LOCK_plugin);

  size_t binlog_index = 0;
  bool found_binlog = false;
  /* Identify binary log SE which we need to invoke first. */
  if (type == MYSQL_STORAGE_ENGINE_PLUGIN) {
    for (idx = 0; idx < total; idx++) {
      /* Note index of binlog */
      plugin = plugins[idx];
      if (plugin && (0 == std::strcmp(plugin->name.str, "binlog"))) {
        binlog_index = idx;
        found_binlog = true;
        break;
      }
    }
  }

  for (; *funcs != nullptr; ++funcs) {
    /* Call binlog engine function first. This is required as GTID is generated
    by binlog to be used by othe SE. */
    if (found_binlog) {
      assert(type == MYSQL_STORAGE_ENGINE_PLUGIN);
      plugin = plugins[binlog_index];
      if (plugin && (*funcs)(thd, plugin_int_to_ref(plugin), arg)) goto err;
      plugins[binlog_index] = nullptr;
    }
    for (idx = 0; idx < total; idx++) {
      if (unlikely(version != plugin_array_version)) {
        mysql_mutex_lock(&LOCK_plugin);
        for (size_t i = idx; i < total; i++)
          if (plugins[i] && plugins[i]->state & state_mask)
            plugins[i] = nullptr;
        mysql_mutex_unlock(&LOCK_plugin);
      }
      plugin = plugins[idx];
      /* It will stop iterating on first engine error when "func" returns true
       */
      if (plugin && (*funcs)(thd, plugin_int_to_ref(plugin), arg)) goto err;
    }
  }

  return false;
err:
  return true;
}


// Source: sql_plugin.cc
// Lines 2675-2752
