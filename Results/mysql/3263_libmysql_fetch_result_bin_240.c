static void fetch_result_bin(MYSQL_BIND *param,
                             MYSQL_FIELD *field MY_ATTRIBUTE((unused)),
                             uchar **row) {
  ulong length = net_field_length(row);
  ulong copy_length = std::min(length, param->buffer_length);
  memcpy(param->buffer, (char *)*row, copy_length);
  *param->length = length;
  *param->error = copy_length < length;
  *row += length;
}


// Source: libmysql.cc
// Lines 3316-3325
