static bool test_if_ref(Item_field *left_item, Item *right_item) {
  if (left_item->depended_from)
    return false;  // don't even read join_tab of inner subquery!
  Field *field = left_item->field;
  JOIN_TAB *join_tab = field->table->reginfo.join_tab;
  if (join_tab == nullptr) return false;

  ASSERT_BEST_REF_IN_JOIN_ORDER(join_tab->join());

  // No need to change const test
  if (!field->table->const_table &&
      /* "ref_or_null" implements "x=y or x is null", not "x=y" */
      (join_tab->type() != JT_REF_OR_NULL)) {
    Item *ref_item = part_of_refkey(field->table, &join_tab->ref(), field);
    return (ref_item && ref_item->eq(right_item, true) &&
            ref_lookup_subsumes_comparison(field, right_item));
  }
  return false;  // keep predicate
}


// Source: sql_optimizer.cc
// Lines 8592-8610
