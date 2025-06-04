bool plugin_var_memalloc_session_update(THD *thd, SYS_VAR *var, char **dest,
                                        const char *value)

{
  LIST *old_element = nullptr;
  struct System_variables *vars = &thd->variables;
  DBUG_TRACE;

  if (value) {
    size_t length = strlen(value) + 1;
    LIST *element;
    if (!(element = (LIST *)my_malloc(key_memory_THD_variables,
                                      sizeof(LIST) + length, MYF(MY_WME))))
      return true;
    memcpy(element + 1, value, length);
    value = (const char *)(element + 1);
    vars->dynamic_variables_allocs =
        list_add(vars->dynamic_variables_allocs, element);
  }

  if (*dest) old_element = (LIST *)(*dest - sizeof(LIST));

  if (var)
    var->update(thd, var, (void **)dest, (const void *)&value);
  else
    *dest = const_cast<char *>(value);

  if (old_element) {
    vars->dynamic_variables_allocs =
        list_delete(vars->dynamic_variables_allocs, old_element);
    my_free(old_element);
  }
  return false;
}


// Source: sql_plugin_var.cc
// Lines 119-152
