TABLE_LIST *nest_join(THD *thd, Query_block *select, TABLE_LIST *embedding,
                      mem_root_deque<TABLE_LIST *> *jlist, size_t table_cnt,
                      const char *legend) {
  DBUG_TRACE;

  TABLE_LIST *const ptr = TABLE_LIST::new_nested_join(thd->mem_root, legend,
                                                      embedding, jlist, select);
  if (ptr == nullptr) return nullptr;

  mem_root_deque<TABLE_LIST *> *const embedded_list =
      &ptr->nested_join->join_list;

  for (uint i = 0; i < table_cnt; i++) {
    TABLE_LIST *table = jlist->front();
    jlist->pop_front();
    table->join_list = embedded_list;
    table->embedding = ptr;
    embedded_list->push_back(table);
    if (table->natural_join) ptr->is_natural_join = true;
  }


// Source: sql_parse.cc
// Lines 5881-5900
