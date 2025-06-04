void JOIN::reset() {
  DBUG_TRACE;

  if (!executed) return;

  query_expression()->offset_limit_cnt = (ha_rows)(
      query_block->offset_limit ? query_block->offset_limit->val_uint() : 0ULL);

  group_sent = false;
  recursive_iteration_count = 0;
  executed = false;

  List_iterator<Window> li(query_block->m_windows);
  Window *w;
  while ((w = li++)) {
    w->reset_round();
  }

  if (tmp_tables) {
    for (uint tmp = primary_tables; tmp < primary_tables + tmp_tables; tmp++) {
      (void)qep_tab[tmp].table()->empty_result_table();
    }
  }
  clear_sj_tmp_tables();
  set_ref_item_slice(REF_SLICE_SAVED_BASE);

  if (qep_tab) {
    if (query_block->derived_table_count) clear_corr_derived_tmp_tables();
    /* need to reset ref access state (see EQRefIterator) */
    for (uint i = 0; i < tables; i++) {
      QEP_TAB *const tab = &qep_tab[i];
      /*
        If qep_tab==NULL, we may still have done ref access (to read a const
        table); const tables will not be re-read in the next execution of this
        subquery, so resetting key_err is not needed.
      */
      tab->ref().key_err = true;
      /*
        If the finished execution used "filesort", it may have reset "quick"
        or "condition" when it didn't need them anymore. Restore them for the
        new execution (the new filesort will need them when it starts).
      */
      tab->restore_quick_optim_and_condition();
    }
  }

  /* Reset of sum functions */
  if (sum_funcs) {
    Item_sum *func, **func_ptr = sum_funcs;
    while ((func = *(func_ptr++))) func->clear();
  }


// Source: sql_select.cc
// Lines 1614-1664
