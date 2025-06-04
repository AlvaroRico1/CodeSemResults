ORDER *create_order_from_distinct(THD *thd, Ref_item_array ref_item_array,
                                  ORDER *order_list,
                                  mem_root_deque<Item *> *fields,
                                  bool skip_aggregates,
                                  bool convert_bit_fields_to_long,
                                  bool *all_order_by_fields_used) {
  ORDER *group = nullptr, **prev = &group;

  *all_order_by_fields_used = true;

  for (ORDER *order = order_list; order; order = order->next) {
    if (order->in_field_list) {
      ORDER *ord = (ORDER *)thd->memdup((char *)order, sizeof(ORDER));
      if (!ord) return nullptr;
      *prev = ord;
      prev = &ord->next;
      (*ord->item)->marker = Item::MARKER_DISTINCT_GROUP;
    } else


// Source: sql_optimizer.cc
// Lines 10237-10254
