static void fetch_result_tinyint(MYSQL_BIND *param, MYSQL_FIELD *field,
                                 uchar **row) {
  bool field_is_unsigned = (field->flags & UNSIGNED_FLAG);
  uchar data = **row;
  *(uchar *)param->buffer = data;
  *param->error = param->is_unsigned != field_is_unsigned && data > INT_MAX8;
  (*row)++;
}


// Source: libmysql.cc
// Lines 3241-3248
