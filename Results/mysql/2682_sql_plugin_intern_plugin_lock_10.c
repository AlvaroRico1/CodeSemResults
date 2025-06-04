static plugin_ref intern_plugin_lock(LEX *lex, plugin_ref rc) {
  st_plugin_int *pi = plugin_ref_to_int(rc);
  DBUG_TRACE;

  mysql_mutex_assert_owner(&LOCK_plugin);

  if (pi->state & (PLUGIN_IS_READY | PLUGIN_IS_UNINITIALIZED)) {
    plugin_ref plugin;
#ifdef NDEBUG
    /* built-in plugins don't need ref counting */
    if (!pi->plugin_dl) return pi;

    plugin = pi;
#else
    /*
      For debugging, we do an additional malloc which allows the
      memory manager and/or valgrind to track locked references and
      double unlocks to aid resolving reference counting problems.
    */
    if (!(plugin = (plugin_ref)my_malloc(key_memory_plugin_ref, sizeof(pi),
                                         MYF(MY_WME))))
      return nullptr;

    *plugin = pi;
#endif
    pi->ref_count++;
    DBUG_PRINT("info", ("thd: %p, plugin: \"%s\", ref_count: %d", current_thd,
                        pi->name.str, pi->ref_count));
    if (lex) lex->plugins.push_back(plugin);
    return plugin;
  }


// Source: sql_plugin.cc
// Lines 936-966
