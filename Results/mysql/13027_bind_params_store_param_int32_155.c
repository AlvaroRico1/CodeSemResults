static void store_param_int32(NET *net, MYSQL_BIND *param) {
  int32 value = *(int32 *)param->buffer;
  int4store(net->write_pos, value);
  net->write_pos += 4;
}


// Source: bind_params.cc
// Lines 129-133
