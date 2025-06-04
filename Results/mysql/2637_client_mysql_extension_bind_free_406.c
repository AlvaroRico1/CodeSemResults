void mysql_extension_bind_free(MYSQL_EXTENSION *ext) {
  DBUG_TRACE;
  if (ext->bind_info.n_params) {
    my_free(ext->bind_info.bind);
    for (uint idx = 0; idx < ext->bind_info.n_params; idx++)
      my_free(ext->bind_info.names[idx]);
    my_free(ext->bind_info.names);
  }


// Source: client.cc
// Lines 3235-3242
