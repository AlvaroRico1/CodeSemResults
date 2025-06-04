static void store_param_float(NET *net, MYSQL_BIND *param) {
  float value = *(float *)param->buffer;
  float4store(net->write_pos, value);
  net->write_pos += 4;
}


// Source: bind_params.cc
// Lines 141-145
