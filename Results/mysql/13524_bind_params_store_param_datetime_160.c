static void store_param_datetime(NET *net, MYSQL_BIND *param) {
  MYSQL_TIME *tm = (MYSQL_TIME *)param->buffer;
  net_store_datetime(net, tm);
}


// Source: bind_params.cc
// Lines 218-221
