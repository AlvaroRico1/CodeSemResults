static void fetch_result_short(MYSQL_BIND *param, MYSQL_FIELD *field,
                               uchar **row) {
  bool field_is_unsigned = (field->flags & UNSIGNED_FLAG);
  ushort data = (ushort)sint2korr(*row);
  shortstore(pointer_cast<uchar *>(param->buffer), data);
  *param->error = param->is_unsigned != field_is_unsigned && data > INT_MAX16;
  *row += 2;
}


// Source: libmysql.cc
// Lines 3250-3257
