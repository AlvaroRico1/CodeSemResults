static my_option *construct_help_options(MEM_ROOT *mem_root, st_plugin_int *p) {
  SYS_VAR **opt;
  my_option *opts;
  uint count = EXTRA_OPTIONS;
  DBUG_TRACE;

  for (opt = p->plugin->system_vars; opt && *opt; opt++, count += 2)
    ;

  if (!(opts = (my_option *)mem_root->Alloc(sizeof(my_option) * count)))
    return nullptr;

  memset(opts, 0, sizeof(my_option) * count);

  /**
    some plugin variables (those that don't have PLUGIN_VAR_EXPERIMENTAL flag)
    have their names prefixed with the plugin name. Restore the names here
    to get the correct (not double-prefixed) help text.
    We won't need @@sysvars anymore and don't care about their proper names.
  */
  restore_pluginvar_names(p->system_vars);

  if (construct_options(mem_root, p, opts)) return nullptr;

  return opts;
}


// Source: sql_plugin.cc
// Lines 3466-3491
