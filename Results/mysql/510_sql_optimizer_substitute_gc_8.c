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


// Source: sql_optimizer.cc
// Lines 1007-1027
