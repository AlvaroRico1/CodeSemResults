bool Query_block::setup_group(THD *thd) {
  DBUG_TRACE;
  assert(group_list.elements);

  thd->where = "group statement";

  for (ORDER *group = group_list.first; group; group = group->next) {
    if (find_order_in_list(thd, base_ref_items, get_table_list(), group,
                           &fields, true, false))
      return true;

    Item *item = *group->item;
    if (item->has_aggregation() || item->has_wf()) {
      my_error(ER_WRONG_GROUP_FIELD, MYF(0), (*group->item)->full_name());
      return true;
    }

    else if (item->has_grouping_func()) {
      my_error(ER_WRONG_GROUP_FIELD, MYF(0), "GROUPING function");
      return true;
    }
    if (item->data_type() == MYSQL_TYPE_INVALID &&
        item->propagate_type(thd, MYSQL_TYPE_VARCHAR))
      return true;
  }

  return false;
}


// Source: sql_resolver.cc
// Lines 4576-4603
