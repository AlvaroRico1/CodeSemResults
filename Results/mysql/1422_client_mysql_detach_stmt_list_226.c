void mysql_detach_stmt_list(LIST **stmt_list MY_ATTRIBUTE((unused)),
                            const char *func_name MY_ATTRIBUTE((unused))) {
#ifndef MYSQL_SERVER
  /* Reset connection handle in all prepared statements. */
  LIST *element = *stmt_list;
  char buff[MYSQL_ERRMSG_SIZE];
  DBUG_TRACE;

  snprintf(buff, sizeof(buff) - 1, ER_CLIENT(CR_STMT_CLOSED), func_name);
  for (; element; element = element->next) {
    MYSQL_STMT *stmt = (MYSQL_STMT *)element->data;
    set_stmt_error(stmt, CR_STMT_CLOSED, unknown_sqlstate, buff);
    stmt->mysql = nullptr;
    /* No need to call list_delete for statement here */
  }


// Source: client.cc
// Lines 6990-7004
