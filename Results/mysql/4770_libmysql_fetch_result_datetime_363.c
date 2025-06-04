static void fetch_result_datetime(MYSQL_BIND *param,
                                  MYSQL_FIELD *field MY_ATTRIBUTE((unused)),
                                  uchar **row) {
  MYSQL_TIME *tm = (MYSQL_TIME *)param->buffer;
  read_binary_datetime(tm, row);
}


// Source: libmysql.cc
// Lines 3309-3314
