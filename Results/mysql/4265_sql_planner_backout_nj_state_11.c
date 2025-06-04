void Optimize_table_order::backout_nj_state(const table_map remaining_tables
                                                MY_ATTRIBUTE((unused)),
                                            const JOIN_TAB *tab) {
  assert(remaining_tables & tab->table_ref->map());

  /* Restore the nested join state */
  TABLE_LIST *last_emb = tab->table_ref->embedding;

  for (; last_emb != emb_sjm_nest; last_emb = last_emb->embedding) {
    // Ignore join nests that are not outer joins.
    if (!last_emb->join_cond_optim()) continue;

    NESTED_JOIN *const nest = last_emb->nested_join;

    assert(nest->nj_counter > 0);

    cur_embedding_map |= nest->nj_map;

    bool was_fully_covered = nest->nj_total == nest->nj_counter;

    if (--nest->nj_counter == 0) cur_embedding_map &= ~nest->nj_map;

    if (!was_fully_covered) break;
  }
}


// Source: sql_planner.cc
// Lines 4605-4629
