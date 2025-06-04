void Query_block::delete_unused_merged_columns(
    mem_root_deque<TABLE_LIST *> *tables) {
  DBUG_TRACE;

  for (TABLE_LIST *tl : *tables) {
    if (tl->nested_join == nullptr) continue;
    if (tl->is_merged()) {
      for (Field_translator *transl = tl->field_translation;
           transl < tl->field_translation_end; transl++) {
        Item *const item = transl->item;

        assert(item->fixed);
        if (!item->has_subquery()) continue;

        /*
          All used columns selected from derived tables are already marked
          as such. But unmarked columns may still refer to other columns
          from underlying derived tables, and in that case we cannot
          delete these columns as they share the same items.
          Thus, dive into the expression and mark such columns as "used".
          (This is a bit incorrect, as only a part of its underlying expression
          is "used", but that has no practical meaning.)
        */
        if (!item->is_derived_used() &&
            item->walk(&Item::propagate_derived_used, enum_walk::POSTFIX,
                       nullptr))
          item->walk(&Item::propagate_set_derived_used,
                     enum_walk::SUBQUERY_POSTFIX, nullptr);

        if (!item->is_derived_used()) {
          Item::Cleanup_after_removal_context ctx(this);
          item->walk(&Item::clean_up_after_removal, enum_walk::SUBQUERY_POSTFIX,
                     pointer_cast<uchar *>(&ctx));
          transl->item = nullptr;
        }
      }


// Source: sql_resolver.cc
// Lines 5031-5066
