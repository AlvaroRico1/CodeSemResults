int prepare_schema_table(THD *thd, LEX *lex, Table_ident *table_ident,
                         enum enum_schema_tables schema_table_idx) {
  Query_block *schema_query_block = nullptr;
  DBUG_TRACE;

  switch (schema_table_idx) {
    case SCH_TMP_TABLE_COLUMNS:
    case SCH_TMP_TABLE_KEYS: {
      assert(table_ident);
      TABLE_LIST **query_tables_last = lex->query_tables_last;
      if ((schema_query_block = lex->new_empty_query_block()) == nullptr)
        return 1; /* purecov: inspected */
      if (!schema_query_block->add_table_to_list(thd, table_ident, nullptr, 0,
                                                 TL_READ, MDL_SHARED_READ))
        return 1;
      lex->query_tables_last = query_tables_last;
      break;
    }


// Source: sql_parse.cc
// Lines 2367-2384
