static void print_table_data_vertically(MYSQL_RES *result) {
  MYSQL_ROW cur;
  uint max_length = 0;
  MYSQL_FIELD *field;

  while ((field = mysql_fetch_field(result))) {
    uint length = field->name_length;
    if (length > max_length) max_length = length;
    field->max_length = length;
  }

  mysql_field_seek(result, 0);
  for (uint row_count = 1; (cur = mysql_fetch_row(result)); row_count++) {
    if (interrupted_query) break;
    mysql_field_seek(result, 0);
    tee_fprintf(
        PAGER,
        "*************************** %d. row ***************************\n",
        row_count);

    ulong *lengths = mysql_fetch_lengths(result);

    for (uint off = 0; off < mysql_num_fields(result); off++) {
      field = mysql_fetch_field(result);
      if (column_names)
        tee_fprintf(PAGER, "%*s: ", (int)max_length, field->name);
      if (cur[off]) {
        if (opt_binhex && is_binary_field(field))
          print_as_hex(PAGER, cur[off], lengths[off], lengths[off]);
        else
          tee_write(PAGER, cur[off], lengths[off],
                    MY_PRINT_SPS_0 | MY_PRINT_MB);
        tee_putc('\n', PAGER);
      } else
        tee_fprintf(PAGER, "NULL\n");
    }
  }


// Source: mysql.cc
// Lines 3834-3870
