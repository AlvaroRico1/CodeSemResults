static st_plugin_dl *plugin_dl_insert_or_reuse(st_plugin_dl *plugin_dl) {
  DBUG_TRACE;
  st_plugin_dl *tmp;
  for (st_plugin_dl **it = plugin_dl_array->begin();
       it != plugin_dl_array->end(); ++it) {
    tmp = *it;
    if (!tmp->ref_count) {
      memcpy(tmp, plugin_dl, sizeof(st_plugin_dl));
      return tmp;
    }
  }
  if (plugin_dl_array->push_back(plugin_dl)) return nullptr;
  tmp = plugin_dl_array->back() = static_cast<st_plugin_dl *>(
      memdup_root(&plugin_mem_root, plugin_dl, sizeof(st_plugin_dl)));
  return tmp;
}


// Source: sql_plugin.cc
// Lines 593-608
