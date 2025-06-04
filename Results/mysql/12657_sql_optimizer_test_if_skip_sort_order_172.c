static bool test_if_skip_sort_order(JOIN_TAB *tab, ORDER_with_src &order,
                                    ha_rows select_limit, const bool no_changes,
                                    const Key_map *map, int *order_idx) {
  DBUG_TRACE;
  int ref_key;
  uint ref_key_parts = 0;
  int order_direction = 0;
  uint used_key_parts = 0;
  TABLE *const table = tab->table();
  JOIN *const join = tab->join();
  THD *const thd = join->thd;
  QUICK_SELECT_I *const save_quick = tab->quick();
  int best_key = -1;
  bool set_up_ref_access_to_key = false;
  bool can_skip_sorting = false;  // used as return value
  int changed_key = -1;

  /* Check that we are always called with first non-const table */
  assert((uint)tab->idx() == join->const_tables);

  Plan_change_watchdog watchdog(tab, no_changes);
  *order_idx = -1;
  /* Sorting a single row can always be skipped */
  if (tab->type() == JT_EQ_REF || tab->type() == JT_CONST ||
      tab->type() == JT_SYSTEM) {
    return true;
  }

  /*
    Check if FT index can be used to retrieve result in the required order.
    It is possible if ordering is on the first non-constant table.
  */
  if (!join->order.empty() && join->simple_order) {
    /*
      Check if ORDER is DESC, ORDER BY is a single MATCH function.
    */
    Item_func_match *ft_func = test_if_ft_index_order(order.order);
    /*
      Two possible cases when we can skip sort order:
      1. FT_SORTED must be set(Natural mode, no ORDER BY).
      2. If FT_SORTED flag is not set then
      the engine should support deferred sorting. Deferred sorting means
      that sorting is postponed utill the start of index reading(InnoDB).
      In this case we set FT_SORTED flag here to let the engine know that
      internal sorting is needed.
    */
    if (ft_func && ft_func->ft_handler && ft_func->ordered_result()) {
      /*
        FT index scan is used, so the only additional requirement is
        that ORDER BY MATCH function is the same as the function that
        is used for FT index.
      */
      if (tab->type() == JT_FT &&
          ft_func->eq(tab->position()->key->val, true)) {
        ft_func->set_hints(join, FT_SORTED, select_limit, false);
        return true;
      }
      /*
        No index is used, it's possible to use FT index for ORDER BY if
        LIMIT is present and does not exceed count of the records in FT index
        and there is no WHERE condition since a condition may potentially
        require more rows to be fetch from FT index.
      */
      else if (!tab->condition() && select_limit != HA_POS_ERROR &&
               select_limit <= ft_func->get_count()) {
        /* test_if_ft_index_order() always returns master MATCH function. */
        assert(!ft_func->master);
        /* ref is not set since there is no WHERE condition */
        assert(tab->ref().key == -1);

        /*Make EXPLAIN happy */
        tab->set_type(JT_FT);
        tab->ref().key = ft_func->key;
        tab->ref().key_parts = 0;
        tab->set_index(ft_func->key);
        tab->set_ft_func(ft_func);

        /* Setup FT handler */
        ft_func->set_hints(join, FT_SORTED, select_limit, true);
        ft_func->join_key = true;
        table->file->ft_handler = ft_func->ft_handler;
        return true;
      }
    }
  }


// Source: sql_optimizer.cc
// Lines 1984-2068
