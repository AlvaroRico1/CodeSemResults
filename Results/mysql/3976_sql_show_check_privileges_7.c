bool Sql_cmd_show_table_base::check_privileges(THD *thd) {
  TABLE_LIST *const table = thd->lex->query_tables;

  if (check_table_access(thd, SELECT_ACL, table, false, UINT_MAX, false))
    return true;

  TABLE_LIST *dst_table = table->schema_query_block->table_list.first;
  assert(dst_table != nullptr);

  if (m_temporary) return false;

  if (check_access(thd, SELECT_ACL, dst_table->db, &dst_table->grant.privilege,
                   &dst_table->grant.m_internal, false, false))
    return true; /* Access denied */

  /*
    Check_grant will grant access if there is any column privileges on
    all of the tables thanks to the fourth parameter (bool show_table).
  */
  if (check_grant(thd, SELECT_ACL, dst_table, true, UINT_MAX, false))
    return true; /* Access denied */

  return false;
}


// Source: sql_show.cc
// Lines 271-294
