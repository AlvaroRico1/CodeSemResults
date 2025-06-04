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
      *all_order_by_fields_used = false;
  }

  Mem_root_array<std::pair<Item *, ORDER *>> bit_fields_to_add(thd->mem_root);

  for (Item *&item : VisibleFields(*fields)) {
    if (!item->const_item() && (!skip_aggregates || !item->has_aggregation()) &&
        item->marker != Item::MARKER_DISTINCT_GROUP) {
      /*
        Don't put duplicate columns from the SELECT list into the
        GROUP BY list.
      */
      ORDER *ord_iter;
      for (ord_iter = group; ord_iter; ord_iter = ord_iter->next)
        if ((*ord_iter->item)->eq(item, true)) goto next_item;

      ORDER *ord = (ORDER *)thd->mem_calloc(sizeof(ORDER));
      if (!ord) return nullptr;

      if (item->type() == Item::FIELD_ITEM &&
          item->data_type() == MYSQL_TYPE_BIT && convert_bit_fields_to_long) {
        /*
          Because HEAP tables can't index BIT fields we need to use an
          additional hidden field for grouping because later it will be
          converted to a LONG field. Original field will remain of the
          BIT type and will be returned to a client.
          @note setup_ref_array() needs to account for the extra space.
          @note We need to defer the actual adding to after the loop,
            or we will invalidate the iterator to “fields”.
        */
        Item_field *new_item = new Item_field(thd, (Item_field *)item);
        ord->item = &item;  // Temporary; for the duplicate check above.
        bit_fields_to_add.push_back(std::make_pair(new_item, ord));
      } else if (ref_item_array.is_null()) {
        // No slices are in use, so just use the field from the list.
        ord->item = &item;
      } else {
        /*
          We have here only visible fields, so we can use simple indexing
          of ref_item_array (order in the array and in the list are same)
        */
        ord->item = &ref_item_array[0];
      }
      ord->direction = ORDER_ASC;
      *prev = ord;
      prev = &ord->next;
    }


// Source: sql_optimizer.cc
// Lines 10237-10301
