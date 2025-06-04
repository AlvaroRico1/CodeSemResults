bool substitute_gc(THD *thd, Query_block *query_block, Item *where_cond,
                   ORDER *group_list, ORDER *order) {
  List<Field> indexed_gc;
  Opt_trace_context *const trace = &thd->opt_trace;
  Opt_trace_object trace_wrapper(trace);
  Opt_trace_object subst_gc(trace, "substitute_generated_columns");

  // Collect all GCs that are a part of a key
  for (TABLE_LIST *tl = query_block->leaf_tables; tl; tl = tl->next_leaf) {
    if (tl->table->s->keys == 0) continue;
    for (uint i = 0; i < tl->table->s->fields; i++) {
      Field *fld = tl->table->field[i];
      if (fld->is_gcol() &&
          !(fld->part_of_key.is_clear_all() &&
            fld->part_of_prefixkey.is_clear_all()) &&
          fld->gcol_info->expr_item->can_be_substituted_for_gc()) {
        // Don't check allowed keys here as conditions/group/order use
        // different keymaps for that.
        indexed_gc.push_back(fld);
      }
    }
  }
  // No GC in the tables used in the query
  if (indexed_gc.elements == 0) return false;

  if (where_cond) {
    // Item_func::compile will dereference this pointer, provide valid value.
    uchar i, *dummy = &i;
    where_cond->compile(&Item::gc_subst_analyzer, &dummy,
                        &Item::gc_subst_transformer, (uchar *)&indexed_gc);
    subst_gc.add("resulting_condition", where_cond);
  }

  // An error occur during substitution. Let caller handle it.
  if (thd->is_error()) return false;

  if (!(group_list || order)) return false;
  // Filter out GCs that do not have index usable for GROUP/ORDER
  Field *gc;
  List_iterator<Field> li(indexed_gc);

  while ((gc = li++)) {
    Key_map tkm = gc->part_of_key;
    tkm.intersect(group_list ? gc->table->keys_in_use_for_group_by
                             : gc->table->keys_in_use_for_order_by);
    if (tkm.is_clear_all()) li.remove();
  }
  if (!indexed_gc.elements) return false;

  // Index could be used for ORDER only if there is no GROUP
  ORDER *list = group_list ? group_list : order;
  bool changed = false;
  for (ORDER *ord = list; ord; ord = ord->next) {
    li.rewind();
    if (!(*ord->item)->can_be_substituted_for_gc()) continue;
    while ((gc = li++)) {
      Item_field *const field =
          get_gc_for_expr(*ord->item, gc, gc->result_type());
      if (field != nullptr) {
        changed = true;
        /* Add new field to field list. */
        Item **new_field = query_block->add_hidden_item(field);
        thd->change_item_tree(ord->item, *new_field);
        query_block->hidden_items_from_optimization++;
        break;
      }
    }
  }
  // An error occur during substitution. Let caller handle it.
  if (thd->is_error()) return false;

  if (changed && trace->is_started()) {
    String str;
    Query_block::print_order(
        thd, &str, list,
        enum_query_type(QT_TO_SYSTEM_CHARSET | QT_SHOW_SELECT_NUMBER |
                        QT_NO_DEFAULT_DB));
    subst_gc.add_utf8(group_list ? "resulting_GROUP_BY" : "resulting_ORDER_BY",
                      str.ptr(), str.length());
  }
  return changed;
}


// Source: sql_optimizer.cc
// Lines 1007-1088
