bool plugin_var_memalloc_global_update(THD *thd, SYS_VAR *var, char **dest,
                                       const char *value) {
  char *old_value = *dest;
  DBUG_EXECUTE_IF("simulate_bug_20292712", my_sleep(1000););
  DBUG_TRACE;

  if (value && !(value = my_strdup(key_memory_global_system_variables, value,
                                   MYF(MY_WME))))
    return true;

  var->update(thd, var, (void **)dest, (const void *)&value);

  if (old_value) my_free(old_value);

  return false;
}


// Source: sql_plugin_var.cc
// Lines 65-80
