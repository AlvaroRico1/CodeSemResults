bool STDCALL mysql_stmt_attr_set(MYSQL_STMT *stmt,
                                 enum enum_stmt_attr_type attr_type,
                                 const void *value) {
  switch (attr_type) {
    case STMT_ATTR_UPDATE_MAX_LENGTH:
      stmt->update_max_length = value ? *(const bool *)value : false;
      break;
    case STMT_ATTR_CURSOR_TYPE: {
      ulong cursor_type;
      cursor_type = value ? *static_cast<const ulong *>(value) : 0UL;
      if (cursor_type > (ulong)CURSOR_TYPE_READ_ONLY) goto err_not_implemented;
      stmt->flags = cursor_type;
      break;
    }


// Source: libmysql.cc
// Lines 2029-2042
