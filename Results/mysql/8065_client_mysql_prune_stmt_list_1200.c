static void mysql_prune_stmt_list(MYSQL *mysql) {
  LIST *pruned_list = nullptr;

  while (mysql->stmts) {
    LIST *element = mysql->stmts;
    MYSQL_STMT *stmt;

    mysql->stmts = list_delete(element, element);
    stmt = (MYSQL_STMT *)element->data;
    if (stmt->state != MYSQL_STMT_INIT_DONE) {
      stmt->mysql = nullptr;
      stmt->last_errno = CR_SERVER_LOST;
      my_stpcpy(stmt->last_error, ER_CLIENT(CR_SERVER_LOST));
      my_stpcpy(stmt->sqlstate, unknown_sqlstate);
    } else {
      pruned_list = list_add(pruned_list, element);
    }
  }


// Source: client.cc
// Lines 6953-6970
