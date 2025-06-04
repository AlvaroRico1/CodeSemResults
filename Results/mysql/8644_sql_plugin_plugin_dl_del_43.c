static void plugin_dl_del(const LEX_STRING *dl) {
  DBUG_TRACE;

  mysql_mutex_assert_owner(&LOCK_plugin);

  for (st_plugin_dl **it = plugin_dl_array->begin();
       it != plugin_dl_array->end(); ++it) {
    st_plugin_dl *tmp = *it;
    if (tmp->ref_count &&
        !my_strnncoll(files_charset_info, pointer_cast<uchar *>(dl->str),
                      dl->length, pointer_cast<uchar *>(tmp->dl.str),
                      tmp->dl.length)) {
      /* Do not remove this element, unless no other plugin uses this dll. */
      if (!--tmp->ref_count) {
        free_plugin_mem(tmp);
        memset(tmp, 0, sizeof(st_plugin_dl));
      }
      break;
    }
  }


// Source: sql_plugin.cc
// Lines 873-892
