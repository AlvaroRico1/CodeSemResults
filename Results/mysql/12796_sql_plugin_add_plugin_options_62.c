void add_plugin_options(std::vector<my_option> *options, MEM_ROOT *mem_root) {
  my_option *opt;

  if (!initialized) return;

  for (st_plugin_int **it = plugin_array->begin(); it != plugin_array->end();
       ++it) {
    st_plugin_int *p = *it;

    if (!(opt = construct_help_options(mem_root, p))) continue;

    /* Only options with a non-NULL comment are displayed in help text */
    for (; opt->name; opt++)
      if (opt->comment) options->push_back(*opt);
  }


// Source: sql_plugin.cc
// Lines 3678-3692
