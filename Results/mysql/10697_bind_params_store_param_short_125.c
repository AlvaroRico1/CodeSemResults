static void store_param_short(NET *net, MYSQL_BIND *param) {
  short value = *(short *)param->buffer;
  int2store(net->write_pos, value);
  net->write_pos += 2;
}


// Source: bind_params.cc
// Lines 123-127
