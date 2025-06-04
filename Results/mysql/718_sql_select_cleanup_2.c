void JOIN::cleanup() {
  DBUG_TRACE;

  assert(const_tables <= primary_tables && primary_tables <= tables);

  if (qep_tab || join_tab || best_ref) {
    for (uint i = 0; i < tables; i++) {
      QEP_TAB *qtab;
      TABLE *table;
      if (qep_tab) {
        assert(!join_tab);
        qtab = &qep_tab[i];
        table = qtab->table();
      } else {
        qtab = nullptr;
        table = (join_tab ? &join_tab[i] : best_ref[i])->table();
      }
      if (!table) continue;
      cleanup_table(table);
    }
  } else if (thd->lex->using_hypergraph_optimizer) {
    for (TABLE_LIST *tl = query_block->leaf_tables; tl; tl = tl->next_leaf) {
      cleanup_table(tl->table);
    }
    for (JOIN::TemporaryTableToCleanup cleanup : temp_tables) {
      cleanup_table(cleanup.table);
    }
  }


// Source: sql_select.cc
// Lines 3589-3616
