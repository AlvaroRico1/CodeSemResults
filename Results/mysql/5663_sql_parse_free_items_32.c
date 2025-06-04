void free_items(Item *item) {
  Item *next;
  DBUG_TRACE;
  for (; item; item = next) {
    next = item->next_free;
    item->delete_self();
  }
}


// Source: sql_parse.cc
// Lines 1165-1172
