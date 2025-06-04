static st_plugin_dl *plugin_dl_find(const LEX_STRING *dl) {
  DBUG_TRACE;
  for (st_plugin_dl **it = plugin_dl_array->begin();
       it != plugin_dl_array->end(); ++it) {
    st_plugin_dl *tmp = *it;
    if (tmp->ref_count &&
        !my_strnncoll(files_charset_info, pointer_cast<uchar *>(dl->str),
                      dl->length, pointer_cast<uchar *>(tmp->dl.str),
                      tmp->dl.length))
      return tmp;
  }


// Source: sql_plugin.cc
// Lines 579-589
