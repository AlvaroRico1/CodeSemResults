static void fetch_result_int32(MYSQL_BIND *param,
                               MYSQL_FIELD *field MY_ATTRIBUTE((unused)),
                               uchar **row) {
  bool field_is_unsigned = (field->flags & UNSIGNED_FLAG);
  uint32 data = (uint32)sint4korr(*row);
  longstore(pointer_cast<uchar *>(param->buffer), data);
  *param->error = param->is_unsigned != field_is_unsigned && data > INT_MAX32;
  *row += 4;
}


// Source: libmysql.cc
// Lines 3259-3267
