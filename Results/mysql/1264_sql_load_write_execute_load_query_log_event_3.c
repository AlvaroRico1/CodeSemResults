bool Sql_cmd_load_table::write_execute_load_query_log_event(
    THD *thd, const char *db_arg, const char *table_name_arg,
    bool is_concurrent, enum enum_duplicates duplicates,
    bool transactional_table, int errcode) {
  const char *tbl = table_name_arg;
  const char *tdb = (thd->db().str != nullptr ? thd->db().str : db_arg);
  const String *query = nullptr;
  String string_buf;
  size_t fname_start = 0;
  size_t fname_end = 0;

  if (thd->db().str == nullptr || strcmp(db_arg, thd->db().str)) {
    /*
      If used database differs from table's database,
      prefix table name with database name so that it
      becomes a FQ name.
     */
    string_buf.set_charset(system_charset_info);
    append_identifier(thd, &string_buf, db_arg, strlen(db_arg));
    string_buf.append(".");
  }
  append_identifier(thd, &string_buf, table_name_arg, strlen(table_name_arg));
  tbl = string_buf.c_ptr_safe();
  Load_query_generator gen(thd, &m_exchange, tdb, tbl, is_concurrent,
                           duplicates == DUP_REPLACE, thd->lex->is_ignore());
  query = gen.generate(&fname_start, &fname_end);

  Execute_load_query_log_event e(
      thd, query->ptr(), query->length(), fname_start, fname_end,
      (duplicates == DUP_REPLACE)
          ? binary_log::LOAD_DUP_REPLACE
          : (thd->lex->is_ignore() ? binary_log::LOAD_DUP_IGNORE
                                   : binary_log::LOAD_DUP_ERROR),
      transactional_table, false, false, errcode);

  return mysql_bin_log.write_event(&e);
}


// Source: sql_load.cc
// Lines 691-727
