static void plugin_var_memalloc_free(struct System_variables *vars) {
  LIST *next, *root;
  DBUG_TRACE;
  for (root = vars->dynamic_variables_allocs; root; root = next) {
    next = root->next;
    my_free(root);
  }
  vars->dynamic_variables_allocs = nullptr;
}


// Source: sql_plugin.cc
// Lines 3193-3201
