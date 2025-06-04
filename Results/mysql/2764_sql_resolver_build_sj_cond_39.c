bool Query_block::build_sj_cond(THD *thd, NESTED_JOIN *nested_join,
                                Query_block *subq_query_block,
                                table_map outer_tables_map, Item **sj_cond) {
  if (nested_join->sj_inner_exprs.empty()) {
    // Semi-join materialization requires a key, push a constant integer item
    Item *const_item = new Item_int(1);
    if (const_item == nullptr) return true;
    nested_join->sj_inner_exprs.push_back(const_item);
    nested_join->sj_outer_exprs.push_back(const_item);
  }

  auto ii = nested_join->sj_inner_exprs.begin();
  auto oi = nested_join->sj_outer_exprs.begin();
  while (ii != nested_join->sj_inner_exprs.end() &&
         oi != nested_join->sj_outer_exprs.end()) {
    bool should_remove = false;
    Item *inner = *ii;
    Item *outer = *oi;
    /*
      Ensure that all involved expressions are pulled out after transformation.
      (If they are already out, this is a no-op).
    */
    outer->fix_after_pullout(this, subq_query_block);
    inner->fix_after_pullout(this, subq_query_block);

    Item_func_eq *item_eq = new Item_func_eq(outer, inner);
    if (item_eq == nullptr) return true; /* purecov: inspected */
    Item *predicate = item_eq;
    if (!item_eq->fixed && item_eq->fix_fields(thd, &predicate)) return true;

    // Evaluate if the condition is on const expressions
    if (predicate->const_item() &&
        !(predicate)->walk(&Item::is_non_const_over_literals,
                           enum_walk::POSTFIX, nullptr)) {
      bool cond_value = true;

      /* Push ignore / strict error handler */
      Ignore_error_handler ignore_handler;
      Strict_error_handler strict_handler;
      if (thd->lex->is_ignore())
        thd->push_internal_handler(&ignore_handler);
      else if (thd->is_strict_mode())
        thd->push_internal_handler(&strict_handler);

      bool err = eval_const_cond(thd, predicate, &cond_value);
      /* Pop ignore / strict error handler */
      if (thd->lex->is_ignore() || thd->is_strict_mode())
        thd->pop_internal_handler();

      if (err) return true;

      if (cond_value) {
        /*
          Remove the expression from inner/outer expression list if the
          const condition evalutes to true as Item_cond::fix_fields will
          remove the condition later.
          Do the above if this is not the last expression in the list.
          Semijoin processing expects atleast one inner/outer expression
          in the list if there is a sj_nest present.
        */
        if (nested_join->sj_inner_exprs.size() != 1) {
          should_remove = true;
        }
      } else {
        /*
          Remove all the expressions in inner/outer expression list if
          one of condition evaluates to always false. Add an always false
          condition to semi-join condition.
        */
        nested_join->sj_inner_exprs.clear();
        nested_join->sj_outer_exprs.clear();
        Item *new_item = new Item_func_false();
        if (new_item == nullptr) return true;
        (*sj_cond) = new_item;
        break;
      }


// Source: sql_resolver.cc
// Lines 2401-2476
