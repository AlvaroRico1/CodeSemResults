static void store_param_double(NET *net, MYSQL_BIND *param) {
  double value = *(double *)param->buffer;
  float8store(net->write_pos, value);
  net->write_pos += 8;
}


// Source: bind_params.cc
// Lines 147-151
