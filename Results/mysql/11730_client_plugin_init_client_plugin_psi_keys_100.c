static void init_client_plugin_psi_keys() {
  const char *category = "sql";
  int count;

  count = array_elements(all_client_plugin_mutexes);
  mysql_mutex_register(category, all_client_plugin_mutexes, count);

  count = array_elements(all_client_plugin_memory);
  mysql_memory_register(category, all_client_plugin_memory, count);
}


// Source: client_plugin.cc
// Lines 90-99
