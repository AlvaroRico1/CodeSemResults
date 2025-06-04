void JOIN::set_semijoin_embedding() {
  assert(!query_block->sj_nests.empty());

  JOIN_TAB *const tab_end = join_tab + primary_tables;

  for (JOIN_TAB *tab = join_tab; tab < tab_end; tab++) {
    tab->emb_sj_nest = nullptr;
    for (TABLE_LIST *tl = tab->table_ref; tl->embedding; tl = tl->embedding) {
      if (tl->embedding->is_sj_or_aj_nest()) {
        assert(!tab->emb_sj_nest);
        tab->emb_sj_nest = tl->embedding;
        // Let the up-walk continue, to assert there's no AJ/SJ nest above.
      }
    }
  }


// Source: sql_optimizer.cc
// Lines 5694-5708
