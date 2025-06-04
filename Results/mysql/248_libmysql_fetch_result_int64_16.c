static void fetch_result_int64(MYSQL_BIND *param,
                               MYSQL_FIELD *field MY_ATTRIBUTE((unused)),
                               uchar **row) {
  bool field_is_unsigned = (field->flags & UNSIGNED_FLAG);
  ulonglong data = (ulonglong)sint8korr(*row);
  *param->error = param->is_unsigned != field_is_unsigned && data > LLONG_MAX;
  longlongstore(pointer_cast<uchar *>(param->buffer), data);
  *row += 8;
}


// Source: libmysql.cc
// Lines 3269-3277
