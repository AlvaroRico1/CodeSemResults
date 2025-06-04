void init_client_psi_keys(void) {
  const char *category = "client";
  int count;

  count = static_cast<int>(array_elements(all_client_memory));
  mysql_memory_register(category, all_client_memory, count);
}


// Source: client.cc
// Lines 186-192
