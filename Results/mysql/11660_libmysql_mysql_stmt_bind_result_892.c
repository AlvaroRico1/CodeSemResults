bool STDCALL mysql_stmt_bind_result(MYSQL_STMT *stmt, MYSQL_BIND *my_bind) {
  MYSQL_BIND *param, *end;
  MYSQL_FIELD *field;
  ulong bind_count = stmt->field_count;
  uint param_count = 0;
  DBUG_TRACE;
  DBUG_PRINT("enter", ("field_count: %lu", bind_count));

  if (!bind_count) {
    int errorcode = (int)stmt->state < (int)MYSQL_STMT_PREPARE_DONE
                        ? CR_NO_PREPARE_STMT
                        : CR_NO_STMT_METADATA;
    set_stmt_error(stmt, errorcode, unknown_sqlstate, nullptr);
    return true;
  }


// Source: libmysql.cc
// Lines 3594-3608
