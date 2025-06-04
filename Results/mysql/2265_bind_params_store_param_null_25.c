static void store_param_null(NET *net MY_ATTRIBUTE((unused)), MYSQL_BIND *param,
                             unsigned char *null_pos) {
  uint pos = param->param_number;
  null_pos[pos / 8] |= (uchar)(1 << (pos & 7));
}


// Source: bind_params.cc
// Lines 245-249
