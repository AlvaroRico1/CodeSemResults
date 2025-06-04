static void store_param_type(unsigned char **pos, MYSQL_BIND *param) {
  uint typecode = param->buffer_type | (param->is_unsigned ? 32768 : 0);
  int2store(*pos, typecode);
  *pos += 2;
}


// Source: bind_params.cc
// Lines 97-101
