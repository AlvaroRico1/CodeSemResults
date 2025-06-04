bool Query_block::setup_tables(THD *thd, TABLE_LIST *tables,
                               bool select_insert) {
  DBUG_TRACE;

  assert((select_insert && !tables->next_name_resolution_table) || !tables ||
         (context.table_list && context.first_name_resolution_table));

  leaf_tables = nullptr;
  (void)make_leaf_tables(&leaf_tables, tables);

  TABLE_LIST *first_query_block_table = nullptr;
  if (select_insert) {
    // "insert_table" is needed for remap_tables().
    thd->lex->insert_table = leaf_tables->top_table();

    // Get first table in SELECT part
    first_query_block_table = thd->lex->insert_table->next_local;

    // Then, find the first leaf table
    if (first_query_block_table)
      first_query_block_table = first_query_block_table->first_leaf_table();
  }
  uint tableno = 0;
  leaf_table_count = 0;
  partitioned_table_count = 0;

  for (TABLE_LIST *tr = leaf_tables; tr; tr = tr->next_leaf, tableno++) {
    TABLE *const table = tr->table;
    if (tr == first_query_block_table) {
      /*
        For INSERT ... SELECT command, restart numbering from zero for first
        leaf table from SELECT part of query.
      */
      first_query_block_table = nullptr;
      tableno = 0;
    }
    if (tableno >= MAX_TABLES) {
      my_error(ER_TOO_MANY_TABLES, MYF(0), static_cast<int>(MAX_TABLES));
      return true;
    }
    tr->set_tableno(tableno);
    leaf_table_count++;  // Count the input tables of the query

    if (opt_hints_qb &&        // QB hints initialized
        !tr->opt_hints_table)  // Table hints are not adjusted yet
    {
      tr->opt_hints_table = opt_hints_qb->adjust_table_hints(tr);
    }

    if (table == nullptr) continue;
    assert(table->pos_in_table_list == tr);
    if (!tr->opt_hints_table ||
        // Ignore old index hint processing if new style hints are specified.
        !tr->opt_hints_table->update_index_hint_maps(thd, tr->table)) {
      if (tr->process_index_hints(thd, table)) return true;
    }

    if (table->part_info)  // Count number of partitioned tables
      partitioned_table_count++;
  }


// Source: sql_resolver.cc
// Lines 1127-1186
