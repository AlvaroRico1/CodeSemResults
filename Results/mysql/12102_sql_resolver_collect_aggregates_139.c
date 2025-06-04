static bool collect_aggregates(
    Query_block *select, Item_sum::Collect_grouped_aggregate_info *aggregates) {
  for (Item *select_expr : select->visible_fields()) {
    if (select_expr->walk(&Item::collect_grouped_aggregates,
                          enum_walk::SUBQUERY_PREFIX,
                          pointer_cast<uchar *>(aggregates)))
      return true; /* purecov: inspected */
  }

  if (select->having_cond() != nullptr) {
    if (select->having_cond()->walk(&Item::collect_grouped_aggregates,
                                    enum_walk::SUBQUERY_PREFIX,
                                    pointer_cast<uchar *>(aggregates)))
      return true; /* purecov: inspected */
  }
  // We move the aggregate functions from an implicitly grouped query block to
  // a new derived table, effectively making the existing query block
  // non-grouped. When the grouping is implicit, the ORDER BY is eliminated
  // since the result set has only one row, so skip processing of the
  // order_list.
  assert(select->order_list.elements == 0);

  List_iterator<Window> li(select->m_windows);
  for (Window *w = li++; w != nullptr; w = li++) {
    for (ORDER *it : {w->first_order_by(), w->first_partition_by()}) {
      if (it != nullptr) {
        for (auto ord = it; ord != nullptr; ord = ord->next) {
          if ((*ord->item)
                  ->walk(&Item::collect_grouped_aggregates, enum_walk::PREFIX,
                         pointer_cast<uchar *>(aggregates)))
            return true; /* purecov: inspected */
        }
      }


// Source: sql_resolver.cc
// Lines 5759-5791
