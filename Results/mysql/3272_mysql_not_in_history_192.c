static int not_in_history(const char *line) {
  HIST_ENTRY *oldhist = history_get(history_length);

  if (oldhist == nullptr) return 1;
  if (strcmp(oldhist->line, line) == 0) return 0;
  return 1;
}


// Source: mysql.cc
// Lines 2702-2708
