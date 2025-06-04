void get_partial_join_cost(JOIN *join, uint n_tables, double *cost_arg,
                           double *rowcount_arg) {
  double rowcount = 1.0;
  double cost = 0.0;
  const Cost_model_server *const cost_model = join->cost_model();

  for (uint i = join->const_tables; i < n_tables + join->const_tables; i++) {
    POSITION *const pos = join->best_positions + i;

    if (pos->rows_fetched > 0.0) {
      rowcount *= pos->rows_fetched;
      cost += pos->read_cost + cost_model->row_evaluate_cost(rowcount);
      rowcount *= pos->filter_effect;
    }
  }


// Source: sql_planner.cc
// Lines 2406-2420
