static void calculate_materialization_costs(JOIN *join, TABLE_LIST *sj_nest,
                                            uint n_tables,
                                            Semijoin_mat_optimize *sjm) {
  double mat_cost;           // Estimated cost of materialization
  double mat_rowcount;       // Estimated row count before duplicate removal
  double distinct_rowcount;  // Estimated rowcount after duplicate removal
  mem_root_deque<Item *> *inner_expr_list;

  if (sj_nest) {
    /*
      get_partial_join_cost() assumes a regular join, which is correct when
      we optimize a sj-materialization nest (always executed as regular
      join).
    */
    get_partial_join_cost(join, n_tables, &mat_cost, &mat_rowcount);
    n_tables += join->const_tables;
    inner_expr_list = &sj_nest->nested_join->sj_inner_exprs;
  } else {
    mat_cost = join->best_read;
    mat_rowcount = static_cast<double>(join->best_rowcount);
    inner_expr_list = &join->query_block->fields;
  }

  /*
    Adjust output cardinality estimates. If the subquery has form

    ... oe IN (SELECT t1.colX, t2.colY, func(X,Y,Z) )

    then the number of distinct output record combinations has an
    upper bound of product of number of records matching the tables
    that are used by the SELECT clause.
    TODO:
    We can get a more precise estimate if we
     - use rec_per_key cardinality estimates. For simple cases like
     "oe IN (SELECT t.key ...)" it is trivial.
     - Functional dependencies between the tables in the semi-join
     nest (the payoff is probably less here?)
  */
  {
    for (uint i = 0; i < n_tables; i++) {
      JOIN_TAB *const tab = join->best_positions[i].table;
      join->map2table[tab->table_ref->tableno()] = tab;
    }


// Source: sql_optimizer.cc
// Lines 10487-10529
