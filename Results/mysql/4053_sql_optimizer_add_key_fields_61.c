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


// Source: sql_optimizer.cc
// Lines 7059-7094
