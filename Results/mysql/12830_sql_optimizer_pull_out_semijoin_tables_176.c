static bool pull_out_semijoin_tables(JOIN *join) {
  DBUG_TRACE;

  assert(!join->query_block->sj_nests.empty());

  Opt_trace_context *const trace = &join->thd->opt_trace;
  Opt_trace_object trace_wrapper(trace);
  Opt_trace_array trace_pullout(trace, "pulled_out_semijoin_tables");

  /* Try pulling out tables from each semi-join nest */
  for (auto sj_list_it = join->query_block->sj_nests.begin();
       sj_list_it != join->query_block->sj_nests.end();) {
    TABLE_LIST *sj_nest = *sj_list_it;
    if (sj_nest->is_aj_nest()) {
      ++sj_list_it;
      continue;
    }
    table_map pulled_tables = 0;
    /*
      Calculate set of tables within this semi-join nest that have
      other dependent tables. They cannot be pulled out. For example, with
      t1 SEMIJOIN (t2 LEFT JOIN t3 ON ...) ON t1.a=t2.pk,
      t2 cannot be pulled out because t3 depends on it.
    */
    table_map dep_tables = 0;
    for (TABLE_LIST *tbl : sj_nest->nested_join->join_list) {
      if (tbl->dep_tables & sj_nest->nested_join->used_tables)
        dep_tables |= tbl->dep_tables;
    }
    /*
      Find which tables we can pull out based on key dependency data.
      Note that pulling one table out can allow us to pull out some
      other tables too.
    */
    bool pulled_a_table;
    do {
      pulled_a_table = false;
      for (TABLE_LIST *tbl : sj_nest->nested_join->join_list) {
        if (tbl->table && !(pulled_tables & tbl->map()) &&
            !(dep_tables & tbl->map())) {
          if (find_eq_ref_candidate(
                  tbl, sj_nest->nested_join->used_tables & ~pulled_tables)) {
            pulled_a_table = true;
            pulled_tables |= tbl->map();
            Opt_trace_object(trace).add_utf8_table(tbl).add(
                "functionally_dependent", true);
            /*
              Pulling a table out of uncorrelated subquery in general makes
              it correlated. See the NOTE to this function.
            */
            sj_nest->nested_join->sj_corr_tables |= tbl->map();
            sj_nest->nested_join->sj_depends_on |= tbl->map();
          }
        }
      }
    } while (pulled_a_table);

    /*
      Move the pulled out TABLE_LIST elements to the parents.
    */
    sj_nest->nested_join->used_tables &= ~pulled_tables;
    sj_nest->nested_join->not_null_tables &= ~pulled_tables;

    /* sj_inner_tables is a copy of nested_join->used_tables */
    sj_nest->sj_inner_tables = sj_nest->nested_join->used_tables;

    bool remove = false;
    if (pulled_tables) {
      mem_root_deque<TABLE_LIST *> *upper_join_list =
          (sj_nest->embedding != nullptr)
              ? &sj_nest->embedding->nested_join->join_list
              : &join->query_block->top_join_list;

      Prepared_stmt_arena_holder ps_arena_holder(join->thd);

      for (auto child_li = sj_nest->nested_join->join_list.begin();
           child_li != sj_nest->nested_join->join_list.end();) {
        TABLE_LIST *tbl = *child_li;
        if (tbl->table && !(sj_nest->nested_join->used_tables & tbl->map())) {
          /*
            Pull the table up in the same way as simplify_joins() does:
            update join_list and embedding pointers but keep next[_local]
            pointers.
          */
          child_li = sj_nest->nested_join->join_list.erase(child_li);

          upper_join_list->push_back(tbl);

          tbl->join_list = upper_join_list;
          tbl->embedding = sj_nest->embedding;
        } else {
          ++child_li;
        }
      }


// Source: sql_optimizer.cc
// Lines 6386-6479
