static void print_table_data(MYSQL_RES *result) {
  String separator(256);
  MYSQL_ROW cur;
  MYSQL_FIELD *field;
  bool *num_flag;
  size_t sz;

  sz = sizeof(bool) * mysql_num_fields(result);
  num_flag = (bool *)my_safe_alloca(sz, MAX_ALLOCA_SIZE);
  if (column_types_flag) {
    print_field_types(result);
    if (!mysql_num_rows(result)) return;
    mysql_field_seek(result, 0);
  }
  separator.copy("+", 1, charset_info);
  while ((field = mysql_fetch_field(result))) {
    size_t length = column_names ? field->name_length : 0;
    if (quick)
      length = max<size_t>(length, field->length);
    else
      length = max<size_t>(length, field->max_length);
    if (length < 4 && !IS_NOT_NULL(field->flags))
      length = 4;  // Room for "NULL"
    if (opt_binhex && is_binary_field(field)) length = 2 + length * 2;
    field->max_length = (ulong)length;
    separator.fill(separator.length() + length + 2, '-');
    separator.append('+');
  }


// Source: mysql.cc
// Lines 3577-3604
