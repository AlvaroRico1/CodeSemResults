TABLE_LIST *Query_block::end_nested_join() {
  TABLE_LIST *ptr;
  NESTED_JOIN *nested_join;
  DBUG_TRACE;

  assert(embedding);
  ptr = embedding;
  join_list = ptr->join_list;
  embedding = ptr->embedding;
  nested_join = ptr->nested_join;
  if (nested_join->join_list.size() == 1) {
    TABLE_LIST *embedded = nested_join->join_list.front();
    join_list->pop_front();
    embedded->join_list = join_list;
    embedded->embedding = embedding;
    join_list->push_front(embedded);
    ptr = embedded;
  } else if (nested_join->join_list.empty()) {
    join_list->pop_front();
    ptr = nullptr;  // return value
  }


// Source: sql_parse.cc
// Lines 5854-5874
