static st_plugin_int *plugin_insert_or_reuse(st_plugin_int *plugin) {
  DBUG_TRACE;
  st_plugin_int *tmp;
  /* During server bootstrap, don't reuse free slot. In case some early plugin
  load like key_ring fails, an user plugin could occupy that empty slot and
  get installed before mandatory plugins like PFS. This will cause issue if
  the plugin has dependency on PFS like creating dynamic PFS table. This issue
  is observed during clone plugin testing. */
  const bool reuse_free_slot = (get_server_state() != SERVER_BOOTING);

  if (reuse_free_slot) {
    for (st_plugin_int **it = plugin_array->begin(); it != plugin_array->end();
         ++it) {
      tmp = *it;
      if (tmp->state == PLUGIN_IS_FREED) {
        *tmp = std::move(*plugin);
        return tmp;
      }
    }
  }


// Source: sql_plugin.cc
// Lines 992-1011
