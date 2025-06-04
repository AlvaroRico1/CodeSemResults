static void fetch_result_time(MYSQL_BIND *param,
                              MYSQL_FIELD *field MY_ATTRIBUTE((unused)),
                              uchar **row) {
  MYSQL_TIME *tm = (MYSQL_TIME *)param->buffer;
  read_binary_time(tm, row);
}


// Source: libmysql.cc
// Lines 3295-3300
