static void fix_tables_after_pullout(Query_block *parent_query_block,
                                     Query_block *removed_query_block,
                                     TABLE_LIST *tr, uint table_adjust,
                                     table_map lateral_deps) {
  if (tr->is_merged()) {
    // Update select list of merged derived tables:
    for (Field_translator *transl = tr->field_translation;
         transl < tr->field_translation_end; transl++) {
      assert(transl->item->fixed);
      transl->item->fix_after_pullout(parent_query_block, removed_query_block);
    }
    // Update used table info for the WHERE clause of the derived table
    assert(!tr->derived_where_cond || tr->derived_where_cond->fixed);
    if (tr->derived_where_cond)
      tr->derived_where_cond->fix_after_pullout(parent_query_block,
                                                removed_query_block);
  }


// Source: sql_resolver.cc
// Lines 2238-2254
