static void fetch_result_double(MYSQL_BIND *param,
                                MYSQL_FIELD *field MY_ATTRIBUTE((unused)),
                                uchar **row) {
  double value = float8get(*row);
  doublestore(pointer_cast<uchar *>(param->buffer), value);
  *row += 8;
}


// Source: libmysql.cc
// Lines 3287-3293
