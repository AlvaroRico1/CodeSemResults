static void store_param_int64(NET *net, MYSQL_BIND *param) {
  longlong value = *(longlong *)param->buffer;
  int8store(net->write_pos, value);
  net->write_pos += 8;
}


// Source: bind_params.cc
// Lines 135-139
