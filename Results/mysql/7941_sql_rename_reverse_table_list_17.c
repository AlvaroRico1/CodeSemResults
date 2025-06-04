static TABLE_LIST *reverse_table_list(TABLE_LIST *table_list) {
  TABLE_LIST *prev = nullptr;

  while (table_list) {
    TABLE_LIST *next = table_list->next_local;
    table_list->next_local = prev;
    prev = table_list;
    table_list = next;
  }


// Source: sql_rename.cc
// Lines 516-524
