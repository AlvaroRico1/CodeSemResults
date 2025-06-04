bool Query_block::apply_local_transforms(THD *thd, bool prune) {
  DBUG_TRACE;

  assert(first_execution);

  /*
    If query block contains one or more merged derived tables/views,
    walk through lists of columns in select lists and remove unused columns.
  */
  if (derived_table_count) delete_unused_merged_columns(&top_join_list);

  for (Query_expression *unit = first_inner_query_expression(); unit;
       unit = unit->next_query_expression()) {
    for (Query_block *sl = unit->first_query_block(); sl;
         sl = sl->next_query_block()) {
      // Prune all subqueries, regardless of passed argument
      if (sl->apply_local_transforms(thd, true)) return true;
    }
    if (unit->fake_query_block &&
        unit->fake_query_block->apply_local_transforms(thd, false))
      return true;
  }

  // Convert all outer joins to inner joins if possible
  if (simplify_joins(thd, &top_join_list, true, false, &m_where_cond))
    return true;
  if (record_join_nest_info(&top_join_list)) return true;
  build_bitmap_for_nested_joins(&top_join_list, 0);

  /*
    Here are the reasons why we do the following check here (i.e. late).
    * setup_fields () may have done split_sum_func () on aggregate items of
    the SELECT list, so for reliable comparison of the ORDER BY list with
    the SELECT list, we need to wait until split_sum_func() is done with
    the ORDER BY list.
    * we get resolved expressions "most of the time", which is always a good
    thing. Some outer references may not be resolved, though.
    * we need nested_join::used_tables, and this member is set in
    simplify_joins()
    * simplify_joins() does outer-join-to-inner conversion, which increases
    opportunities for functional dependencies (weak-to-strong, which is
    unusable, becomes strong-to-strong).
    * check_only_full_group_by() is dependent on processing done by
    simplify_joins() (for example it uses the value of
    Query_block::outer_join).

    The drawback is that the checks are after resolve_subquery(), so can
    meet strange "internally added" items.

    Note that when we are creating a view, simplify_joins() doesn't run so
    check_only_full_group_by() cannot run, any error will be raised only
    when the view is later used (SELECTed...)
  */
  if ((is_distinct() || is_grouped()) &&
      (thd->variables.sql_mode & MODE_ONLY_FULL_GROUP_BY) &&
      check_only_full_group_by(thd))
    return true;

  /*
    Prune partitions for all query blocks after query block merging, if
    pruning is wanted.
  */
  if (partitioned_table_count && prune) {
    for (TABLE_LIST *tbl = leaf_tables; tbl; tbl = tbl->next_leaf) {
      /*
        This will only prune constant conditions, which will be used for
        lock pruning.
      */
      if (prune_partitions(thd, tbl->table, this,
                           tbl->join_cond() ? tbl->join_cond() : m_where_cond))
        return true; /* purecov: inspected */

      if (tbl->table->all_partitions_pruned_away &&
          !tbl->is_inner_table_of_outer_join())
        set_empty_query();
    }
  }


// Source: sql_resolver.cc
// Lines 724-800
