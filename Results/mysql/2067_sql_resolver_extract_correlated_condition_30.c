static bool extract_correlated_condition(THD *thd, Item **cond,
                                         Item **correlated_cond) {
  Item_cond *or_condition = down_cast<Item_cond *>(*cond);
  Item *cor_pred = nullptr;
  bool found = false;
  for (Item &item : *or_condition->argument_list()) {
    Mem_root_array<Item *> cond_parts(thd->mem_root);
    ExtractConditions(&item, &cond_parts);  // all elements AND'ed
    found = false;
    for (Item *pred : cond_parts) {
      // Check if we have a correlated condition that is present in all the
      // arguments to this OR condition. Only then we can extract it.
      if (pred->is_outer_reference()) {
        // If the correlated condition itself is disjuntive, we reject.
        if (pred->type() == Item::COND_ITEM) return true;
        // If this is the first argument to the OR condition, we need to be
        // finding this correlated condition in all other arguments of the OR
        // condition
        if (cor_pred == nullptr) cor_pred = pred;
        // If it is not the first argument to the OR condition, we already
        // have a predicate with us that we need to look for in this argument.
        // So, continue to search until we find it.
        else if (!cor_pred->eq(pred, false))
          continue;
        found = true;
        if (check_predicate_and_args(cor_pred)) return true;
        break;
      }
    }
    if (!found) return true;
  }


// Source: sql_resolver.cc
// Lines 6921-6951
