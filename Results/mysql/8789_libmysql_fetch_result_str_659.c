static void fetch_result_str(MYSQL_BIND *param,
                             MYSQL_FIELD *field MY_ATTRIBUTE((unused)),
                             uchar **row) {
  ulong length = net_field_length(row);
  ulong copy_length = std::min(length, param->buffer_length);
  memcpy(param->buffer, (char *)*row, copy_length);
  /* Add an end null if there is room in the buffer */
  if (copy_length != param->buffer_length)
    ((uchar *)param->buffer)[copy_length] = '\0';
  *param->length = length; /* return total length */
  *param->error = copy_length < length;
  *row += length;
}


// Source: libmysql.cc
// Lines 3327-3339
