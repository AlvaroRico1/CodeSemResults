void JOIN::destroy() {
  cond_equal = nullptr;

  set_plan_state(NO_PLAN);

  if (qep_tab) {
    assert(!join_tab);
    for (uint i = 0; i < tables; i++) {
      TABLE *table = qep_tab[i].table();
      if (table != nullptr) {
        // These were owned by the root iterator, which we just destroyed.
        // Keep filesort_free_buffers() from trying to call CleanupAfterQuery()
        // on them.
        table->sorting_iterator = nullptr;
        table->duplicate_removal_iterator = nullptr;
      }
      qep_tab[i].cleanup();
    }
  } else {
    // Same, for hypergraph queries.
    for (TABLE_LIST *tl = query_block->leaf_tables; tl; tl = tl->next_leaf) {
      TABLE *table = tl->table;
      if (table != nullptr) {
        table->sorting_iterator = nullptr;
        table->duplicate_removal_iterator = nullptr;
      }
    }


// Source: sql_select.cc
// Lines 1701-1727
