int STDCALL mysql_session_track_get_next(MYSQL *mysql,
                                         enum enum_session_state_type type,
                                         const char **data, size_t *length) {
  STATE_INFO *info = STATE_DATA(mysql);
  int ret;

  if (!info || !IS_SESSION_STATE_TYPE(type) ||
      !(info->info_list[type].current_node))
    return get_data_and_length(nullptr, data, length);

  ret = get_data_and_length(info->info_list[type].current_node, data, length);

  info->info_list[type].current_node =
      list_rest(info->info_list[type].current_node);

  return ret;
}


// Source: client.cc
// Lines 8538-8554
