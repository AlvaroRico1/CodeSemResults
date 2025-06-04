static void restore_pluginvar_names(sys_var *first) {
  for (sys_var *var = first; var; var = var->next) {
    sys_var_pluginvar *pv = var->cast_pluginvar();
    pv->plugin_var->name = pv->orig_pluginvar_name;
  }


// Source: sql_plugin.cc
// Lines 2917-2921
