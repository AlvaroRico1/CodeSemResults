static void skip_result_string(MYSQL_BIND *param MY_ATTRIBUTE((unused)),
                               MYSQL_FIELD *field, uchar **row)

{
  ulong length = net_field_length(row);
  (*row) += length;
  if (field->max_length < length) field->max_length = length;
}


// Source: libmysql.cc
// Lines 3363-3370
