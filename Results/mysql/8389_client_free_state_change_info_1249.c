static void free_state_change_info(MYSQL_EXTENSION *ext) {
  STATE_INFO *info;
  int i;

  if (ext)
    info = &ext->state_change;
  else
    return;

  for (i = SESSION_TRACK_SYSTEM_VARIABLES; i <= SESSION_TRACK_END; i++) {
    if (list_length(info->info_list[i].head_node) != 0) {
      list_free(info->info_list[i].head_node, (uint)0);
    }
  }
  memset(info, 0, sizeof(STATE_INFO));
}


// Source: client.cc
// Lines 680-695
