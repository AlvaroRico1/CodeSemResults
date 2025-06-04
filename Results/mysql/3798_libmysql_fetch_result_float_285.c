static void fetch_result_float(MYSQL_BIND *param,
                               MYSQL_FIELD *field MY_ATTRIBUTE((unused)),
                               uchar **row) {
  float value = float4get(*row);
  floatstore(pointer_cast<uchar *>(param->buffer), value);
  *row += 4;
}


// Source: libmysql.cc
// Lines 3279-3285
