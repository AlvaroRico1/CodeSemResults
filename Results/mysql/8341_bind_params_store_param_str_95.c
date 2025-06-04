static void store_param_str(NET *net, MYSQL_BIND *param) {
  /* param->length is always set in mysql_stmt_bind_param */
  ulong length = *param->length;
  uchar *to = net_store_length(net->write_pos, length);
  memcpy(to, param->buffer, length);
  net->write_pos = to + length;
}


// Source: bind_params.cc
// Lines 223-229
