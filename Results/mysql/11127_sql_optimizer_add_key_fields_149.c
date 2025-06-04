bool add_key_fields(THD *thd, JOIN *join, Key_field **key_fields,
                    uint *and_level, Item *cond, table_map usable_tables,
                    SARGABLE_PARAM **sargables) {
  assert(cond->is_bool_func());

  if (cond->type() == Item_func::COND_ITEM) {
    List_iterator_fast<Item> li(*((Item_cond *)cond)->argument_list());
    Key_field *org_key_fields = *key_fields;

    if (down_cast<Item_cond *>(cond)->functype() == Item_func::COND_AND_FUNC) {
      Item *item;
      while ((item = li++)) {
        if (add_key_fields(thd, join, key_fields, and_level, item,
                           usable_tables, sargables))
          return true;
      }
      for (; org_key_fields != *key_fields; org_key_fields++)
        org_key_fields->level = *and_level;
    } else {
      (*and_level)++;
      if (add_key_fields(thd, join, key_fields, and_level, li++, usable_tables,
                         sargables))
        return true;
      Item *item;
      while ((item = li++)) {
        Key_field *start_key_fields = *key_fields;
        (*and_level)++;
        if (add_key_fields(thd, join, key_fields, and_level, item,
                           usable_tables, sargables))
          return true;
        *key_fields = merge_key_fields(org_key_fields, start_key_fields,
                                       *key_fields, ++(*and_level));
      }
    }
    return false;
  }

  /*
    Subquery optimization: Conditions that are pushed down into subqueries
    are wrapped into Item_func_trig_cond. We process the wrapped condition
    but need to set cond_guard for Key_use elements generated from it.
  */
  if (cond->type() == Item::FUNC_ITEM &&
      down_cast<Item_func *>(cond)->functype() == Item_func::TRIG_COND_FUNC) {
    Item *const cond_arg = down_cast<Item_func *>(cond)->arguments()[0];
    if (join->group_list.empty() && join->order.empty() &&
        join->query_expression()->item &&
        join->query_expression()->item->substype() == Item_subselect::IN_SUBS &&
        !join->query_expression()->is_union()) {
      Key_field *save = *key_fields;
      if (add_key_fields(thd, join, key_fields, and_level, cond_arg,
                         usable_tables, sargables))
        return true;
      // Indicate that this ref access candidate is for subquery lookup:
      for (; save != *key_fields; save++)
        save->cond_guard = ((Item_func_trig_cond *)cond)->get_trig_var();
    }


// Source: sql_optimizer.cc
// Lines 7059-7115
