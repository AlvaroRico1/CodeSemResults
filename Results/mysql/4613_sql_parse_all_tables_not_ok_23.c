bool all_tables_not_ok(THD *thd, TABLE_LIST *tables) {
  Rpl_filter *rpl_filter = thd->rli_slave->rpl_filter;

  return rpl_filter->is_on() && tables && !thd->sp_runtime_ctx &&
         !rpl_filter->tables_ok(thd->db().str, tables);
}


// Source: sql_parse.cc
// Lines 265-270
