bool JOIN::alloc_func_list() {
  uint func_count, group_parts;
  DBUG_TRACE;

  func_count = tmp_table_param.sum_func_count;
  /*
    If we are using rollup, we need a copy of the summary functions for
    each level
  */
  if (rollup_state != RollupState::NONE) func_count *= (send_group_parts + 1);

  group_parts = send_group_parts;
  /*
    If distinct, reserve memory for possible
    disctinct->group_by optimization
  */
  if (select_distinct) {
    group_parts += CountVisibleFields(*fields);
    /*
      If the ORDER clause is specified then it's possible that
      it also will be optimized, so reserve space for it too
    */
    if (!order.empty()) {
      ORDER *ord;
      for (ord = order.order; ord; ord = ord->next) group_parts++;
    }


// Source: sql_select.cc
// Lines 3959-3984
