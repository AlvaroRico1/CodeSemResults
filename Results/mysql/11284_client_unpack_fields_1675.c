MYSQL_FIELD *unpack_fields(MYSQL *mysql, MYSQL_ROWS *data, MEM_ROOT *alloc,
                           uint fields, bool default_value,
                           uint server_capabilities) {
  MYSQL_ROWS *row;
  MYSQL_FIELD *field, *result;
  DBUG_TRACE;

  field = result = (MYSQL_FIELD *)alloc->Alloc((uint)sizeof(*field) * fields);
  if (!result) {
    set_mysql_error(mysql, CR_OUT_OF_MEMORY, unknown_sqlstate);
    return nullptr;
  }
  memset(field, 0, sizeof(MYSQL_FIELD) * fields);
  for (row = data; row; row = row->next, field++) {
    /* fields count may be wrong */
    if (field < result || static_cast<uint>(field - result) >= fields) {
      return nullptr;
    }
    if (unpack_field(mysql, alloc, default_value, server_capabilities, row,
                     field)) {
      return nullptr;
    }
  }
  return result;
}


// Source: client.cc
// Lines 2430-2454
