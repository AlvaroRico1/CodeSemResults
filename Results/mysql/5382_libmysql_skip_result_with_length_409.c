static void skip_result_with_length(MYSQL_BIND *param MY_ATTRIBUTE((unused)),
                                    MYSQL_FIELD *field MY_ATTRIBUTE((unused)),
                                    uchar **row)

{
  ulong length = net_field_length(row);
  (*row) += length;
}


// Source: libmysql.cc
// Lines 3354-3361
