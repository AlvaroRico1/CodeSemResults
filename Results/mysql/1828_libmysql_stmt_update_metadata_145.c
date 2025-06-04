static void stmt_update_metadata(MYSQL_STMT *stmt, MYSQL_ROWS *data) {
  MYSQL_BIND *my_bind, *end;
  MYSQL_FIELD *field;
  uchar *null_ptr, bit;
  uchar *row = (uchar *)data->data;
#ifndef NDEBUG
  uchar *row_end = row + data->length;
#endif

  null_ptr = row;
  row += (stmt->field_count + 9) / 8; /* skip null bits */
  bit = 4;                            /* first 2 bits are reserved */

  /* Go through all fields and calculate metadata */
  for (my_bind = stmt->bind, end = my_bind + stmt->field_count,
      field = stmt->fields;
       my_bind < end; my_bind++, field++) {
    if (!(*null_ptr & bit)) (*my_bind->skip_result)(my_bind, field, &row);
    assert(row <= row_end);
    if (!((bit <<= 1) & 255)) {
      bit = 1; /* To next uchar */
      null_ptr++;
    }
  }
}


// Source: libmysql.cc
// Lines 3888-3912
