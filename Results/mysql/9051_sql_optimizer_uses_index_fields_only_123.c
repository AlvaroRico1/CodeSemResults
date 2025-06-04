bool uses_index_fields_only(Item *item, TABLE *tbl, uint keyno,
                            bool other_tbls_ok) {
  // Restrictions b and c.
  if (item->has_stored_program() || item->has_subquery()) return false;

  // No table fields in const items
  if (item->const_for_execution()) return true;

  const Item::Type item_type = item->type();

  switch (item_type) {
    case Item::FUNC_ITEM: {
      Item_func *item_func = (Item_func *)item;
      const Item_func::Functype func_type = item_func->functype();

      if (func_type == Item_func::TRIG_COND_FUNC ||  // Restriction a.
          func_type == Item_func::DD_INTERNAL_FUNC)  // Restriction d.
        return false;

      /* This is a function, apply condition recursively to arguments */
      if (item_func->argument_count() > 0) {
        Item **item_end =
            (item_func->arguments()) + item_func->argument_count();
        for (Item **child = item_func->arguments(); child != item_end;
             child++) {
          if (!uses_index_fields_only(*child, tbl, keyno, other_tbls_ok))
            return false;
        }
      }
      return true;
    }


// Source: sql_optimizer.cc
// Lines 6145-6175
