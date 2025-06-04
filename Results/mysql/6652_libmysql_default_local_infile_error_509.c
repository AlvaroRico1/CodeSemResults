static int default_local_infile_error(void *ptr, char *error_msg,
                                      uint error_msg_len) {
  default_local_infile_data *data = (default_local_infile_data *)ptr;
  if (data) /* If not error on open */
  {
    strmake(error_msg, data->error_msg, error_msg_len);
    return data->error_num;
  }
  /* This can only happen if we got error on malloc of handle */
  my_stpcpy(error_msg, ER_CLIENT(CR_OUT_OF_MEMORY));
  return CR_OUT_OF_MEMORY;
}


// Source: libmysql.cc
// Lines 615-626
