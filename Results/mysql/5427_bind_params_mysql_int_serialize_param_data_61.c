bool mysql_int_serialize_param_data(NET *net, unsigned int param_count,
                                    MYSQL_BIND *params, const char **names,
                                    unsigned long n_param_sets,
                                    uchar **ret_data, ulong *ret_length,
                                    uchar send_types_to_server,
                                    bool send_named_params,
                                    bool send_parameter_set_count) {
  uint null_count;
  MYSQL_BIND *param, *param_end;
  const char **names_ptr = names;
  unsigned char *null_pos;
  DBUG_TRACE;

  assert(net->vio);
  net_clear(net, true); /* Sets net->write_pos */

  if (send_named_params) {
    /* send the number of params */
    my_realloc_str(net, net_length_size(param_count));
    uchar *to = net_store_length(net->write_pos, param_count);
    net->write_pos = to;

    /* also send the number of parameter data sets */
    assert(n_param_sets == 1);  // reserved for now
    if (send_parameter_set_count) {
      my_realloc_str(net, net_length_size(n_param_sets));
      to = net_store_length(net->write_pos, n_param_sets);
      net->write_pos = to;
    }
  }


// Source: bind_params.cc
// Lines 302-331
