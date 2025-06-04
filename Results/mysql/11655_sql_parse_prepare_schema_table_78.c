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
    case SCH_PROFILES:
      /*
        Mark this current profiling record to be discarded.  We don't
        wish to have SHOW commands show up in profiling->
      */
#if defined(ENABLED_PROFILING)
      thd->profiling->discard_current_query();
#endif
      break;
    case SCH_OPTIMIZER_TRACE:
    case SCH_OPEN_TABLES:
    case SCH_ENGINES:
    case SCH_USER_PRIVILEGES:
    case SCH_SCHEMA_PRIVILEGES:
    case SCH_TABLE_PRIVILEGES:
    case SCH_COLUMN_PRIVILEGES:
    default:
      break;
  }

  Query_block *query_block = lex->current_query_block();
  if (make_schema_query_block(thd, query_block, schema_table_idx)) {
    return 1;
  }
  TABLE_LIST *table_list = query_block->table_list.first;
  table_list->schema_query_block = schema_query_block;
  table_list->schema_table_reformed = true;
  return 0;
}


// Source: sql_parse.cc
// Lines 2367-2413
