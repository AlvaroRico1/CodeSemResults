static double accumulate_statement_cost(const LEX *lex) {
  Opt_trace_context *trace = &lex->thd->opt_trace;
  Opt_trace_disable_I_S disable_trace(trace, true);

  double total_cost = 0.0;
  for (const Query_block *query_block = lex->all_query_blocks_list;
       query_block != nullptr;
       query_block = query_block->next_select_in_list()) {
    if (query_block->join == nullptr) continue;

    // Get the cost of this query block.
    double query_block_cost = query_block->join->best_read;

    // If it is a non-cacheable subquery, estimate how many times it
    // needs to be executed, and adjust the cost accordingly.
    const Item_subselect *item = query_block->master_query_expression()->item;
    if (item != nullptr && !query_block->is_cacheable())
      query_block_cost *= calculate_subquery_executions(item, trace);

    total_cost += query_block_cost;
  }

  return total_cost;
}


// Source: sql_select.cc
// Lines 650-673
