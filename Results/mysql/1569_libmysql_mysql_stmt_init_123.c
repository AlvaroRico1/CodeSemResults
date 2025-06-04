MYSQL_STMT *STDCALL mysql_stmt_init(MYSQL *mysql) {
  MYSQL_STMT *stmt;
  DBUG_TRACE;

  if (!(stmt = (MYSQL_STMT *)my_malloc(PSI_NOT_INSTRUMENTED, sizeof(MYSQL_STMT),
                                       MYF(MY_WME | MY_ZEROFILL))) ||
      !(stmt->extension = (MYSQL_STMT_EXT *)my_malloc(
            PSI_NOT_INSTRUMENTED, sizeof(MYSQL_STMT_EXT),
            MYF(MY_WME | MY_ZEROFILL))) ||
      !(stmt->mem_root =
            (MEM_ROOT *)my_malloc(PSI_NOT_INSTRUMENTED, sizeof(MEM_ROOT),
                                  MYF(MY_WME | MY_ZEROFILL))) ||
      !(stmt->result.alloc =
            (MEM_ROOT *)my_malloc(PSI_NOT_INSTRUMENTED, sizeof(MEM_ROOT),
                                  MYF(MY_WME | MY_ZEROFILL)))) {
    set_mysql_error(mysql, CR_OUT_OF_MEMORY, unknown_sqlstate);
    my_free(stmt);
    return nullptr;
  }

  init_alloc_root(PSI_NOT_INSTRUMENTED, stmt->mem_root, 2048, 2048);
  init_alloc_root(PSI_NOT_INSTRUMENTED, stmt->result.alloc, 4096, 4096);
  mysql->stmts = list_add(mysql->stmts, &stmt->list);
  stmt->list.data = stmt;
  stmt->state = MYSQL_STMT_INIT_DONE;
  stmt->mysql = mysql;
  stmt->read_row_func = stmt_read_row_no_result_set;
  stmt->prefetch_rows = DEFAULT_PREFETCH_ROWS;
  my_stpcpy(stmt->sqlstate, not_error_sqlstate);
  /* The rest of statement members was zeroed inside malloc */

  init_alloc_root(PSI_NOT_INSTRUMENTED, &stmt->extension->fields_mem_root, 2048,
                  0);

  return stmt;
}


// Source: libmysql.cc
// Lines 1368-1403
