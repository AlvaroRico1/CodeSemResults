void Query_block::remap_tables(THD *thd) {
  LEX *const lex = thd->lex;
  TABLE_LIST *first_query_block_table = nullptr;
  if (lex->insert_table && lex->insert_table == leaf_tables->top_table()) {
    /*
      For INSERT ... SELECT command, restart numbering from zero for first
      leaf table from SELECT part of query.
    */
    // Get first table in SELECT part
    first_query_block_table = lex->insert_table->next_local;

    // Then, recurse down to get first leaf table
    if (first_query_block_table)
      first_query_block_table = first_query_block_table->first_leaf_table();
  }

  uint tableno = 0;
  for (TABLE_LIST *tl = leaf_tables; tl; tl = tl->next_leaf) {
    // Reset table number after having reached first table after insert table
    if (first_query_block_table == tl) tableno = 0;
    tl->set_tableno(tableno++);
  }
}


// Source: sql_resolver.cc
// Lines 1209-1231
