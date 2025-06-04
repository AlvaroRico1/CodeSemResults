void mysqld_stmt_fetch(THD *thd, Prepared_statement *stmt, ulong num_rows) {
  DBUG_TRACE;
  thd->status_var.com_stmt_fetch++;

  Server_side_cursor *cursor = stmt->cursor;
  if (cursor == nullptr || !cursor->is_open()) {
    my_error(ER_STMT_HAS_NO_OPEN_CURSOR, MYF(0), stmt->id);
    return;
  }

  thd->stmt_arena = &stmt->m_arena;
  Statement_backup stmt_backup;
  stmt_backup.set_thd_to_ps(thd, stmt);

  cursor->fetch(num_rows);

  if (!cursor->is_open()) reset_stmt_params(stmt);

  stmt_backup.restore_thd(thd, stmt);
  thd->stmt_arena = thd;
}


// Source: sql_prepare.cc
// Lines 1968-1988
