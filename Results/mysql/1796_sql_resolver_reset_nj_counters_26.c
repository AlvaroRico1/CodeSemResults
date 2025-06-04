void Query_block::reset_nj_counters(mem_root_deque<TABLE_LIST *> *join_list) {
  DBUG_TRACE;
  if (join_list == nullptr) join_list = &top_join_list;
  for (TABLE_LIST *table : *join_list) {
    NESTED_JOIN *nested_join;
    if ((nested_join = table->nested_join)) {
      nested_join->nj_counter = 0;
      reset_nj_counters(&nested_join->join_list);
    }
  }


// Source: sql_resolver.cc
// Lines 1731-1740
