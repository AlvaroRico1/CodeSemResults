void Query_block::fix_after_pullout(Query_block *parent_query_block,
                                    Query_block *removed_query_block) {
  if (where_cond())
    where_cond()->fix_after_pullout(parent_query_block, removed_query_block);

  /*
    Join conditions can contain an outer reference; and
    derived table merging changes WHERE to a join condition, which thus can
    have an outer reference. So we have to call fix_after_pullout() on join
    conditions. The reference may also be located in a derived table used by
    this subquery. fix_tables_after_pullout() will handle the two cases.
    table_adjust and lateral_deps are 0 because we're not merging these tables
    up.
  */
  for (TABLE_LIST *tr : top_join_list) {
    fix_tables_after_pullout(parent_query_block, removed_query_block, tr,
                             /*table_adjust=*/0, /*lateral_deps=*/0);
  }

  if (having_cond())
    having_cond()->fix_after_pullout(parent_query_block, removed_query_block);

  for (Item *item : visible_fields()) {
    item->fix_after_pullout(parent_query_block, removed_query_block);
  }

  /* Re-resolve ORDER BY and GROUP BY fields */

  for (ORDER *order = order_list.first; order; order = order->next)
    (*order->item)->fix_after_pullout(parent_query_block, removed_query_block);

  for (ORDER *group = group_list.first; group; group = group->next)
    (*group->item)->fix_after_pullout(parent_query_block, removed_query_block);
}


// Source: sql_resolver.cc
// Lines 2336-2369
