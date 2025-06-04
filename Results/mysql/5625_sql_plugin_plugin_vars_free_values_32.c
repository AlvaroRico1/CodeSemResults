static void plugin_vars_free_values(sys_var *vars) {
  DBUG_TRACE;

  for (sys_var *var = vars; var; var = var->next) {
    sys_var_pluginvar *piv = var->cast_pluginvar();
    if (piv &&
        ((piv->plugin_var->flags & PLUGIN_VAR_TYPEMASK) == PLUGIN_VAR_STR) &&
        (piv->plugin_var->flags & PLUGIN_VAR_MEMALLOC)) {
      /* Free the string from global_system_variables. */
      char **valptr = (char **)piv->real_value_ptr(nullptr, OPT_GLOBAL);
      DBUG_PRINT("plugin",
                 ("freeing value for: '%s'  addr: %p", var->name.str, valptr));
      my_free(*valptr);
      *valptr = nullptr;
    }
  }
}


// Source: sql_plugin.cc
// Lines 3141-3157
