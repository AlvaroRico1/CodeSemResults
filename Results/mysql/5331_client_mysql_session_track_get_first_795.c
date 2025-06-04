int STDCALL mysql_session_track_get_first(MYSQL *mysql,
                                          enum enum_session_state_type type,
                                          const char **data, size_t *length) {
  STATE_INFO *info = STATE_DATA(mysql);

  if (!info || !IS_SESSION_STATE_TYPE(type) ||
      !(info->info_list[type].head_node))
    return get_data_and_length(nullptr, data, length);

  info->info_list[type].current_node = info->info_list[type].head_node;

  return mysql_session_track_get_next(mysql, type, data, length);
}


// Source: client.cc
// Lines 8511-8523
