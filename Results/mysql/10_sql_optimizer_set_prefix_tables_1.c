void JOIN::set_prefix_tables() {
  ASSERT_BEST_REF_IN_JOIN_ORDER(this);
  assert(!plan_is_const());
  /*
    The const tables are available together with the first non-const table in
    the join order.
  */
  table_map const initial_tables_map =
      const_table_map | (allow_outer_refs ? OUTER_REF_TABLE_BIT : 0);

  table_map current_tables_map = initial_tables_map;
  table_map prev_tables_map = (table_map)0;
  table_map saved_tables_map = (table_map)0;

  JOIN_TAB *last_non_sjm_tab = nullptr;  // Track the last non-sjm table

  for (uint i = const_tables; i < tables; i++) {
    JOIN_TAB *const tab = best_ref[i];
    if (!tab->table()) continue;
    /*
      Tables that are within SJ-Materialization nests cannot have their
      conditions referring to preceding non-const tables.
       - If we're looking at the first SJM table, reset current_tables_map
         to refer to only allowed tables
      @see Item_equal::get_subst_item()
      @see eliminate_item_equal()
    */
    if (sj_is_materialize_strategy(tab->get_sj_strategy())) {
      const table_map sjm_inner_tables = tab->emb_sj_nest->sj_inner_tables;
      if (!(sjm_inner_tables & current_tables_map)) {
        saved_tables_map = current_tables_map;
        current_tables_map = initial_tables_map;
        prev_tables_map = (table_map)0;
      }

      current_tables_map |= tab->table_ref->map();
      tab->set_prefix_tables(current_tables_map, prev_tables_map);
      prev_tables_map = current_tables_map;

      if (!(sjm_inner_tables & ~current_tables_map)) {
        /*
          At the end of a semi-join materialization nest,
          add non-deterministic expressions to the last table of the nest:
        */
        tab->add_prefix_tables(RAND_TABLE_BIT);

        // Restore the previous map:
        current_tables_map = saved_tables_map;
        prev_tables_map =
            last_non_sjm_tab ? last_non_sjm_tab->prefix_tables() : (table_map)0;
      }
    } else {
      last_non_sjm_tab = tab;
      current_tables_map |= tab->table_ref->map();
      tab->set_prefix_tables(current_tables_map, prev_tables_map);
      prev_tables_map = current_tables_map;
    }
  }


// Source: sql_optimizer.cc
// Lines 4870-4927
