static int default_local_infile_read(void *ptr, char *buf, uint buf_len) {
  int count;
  default_local_infile_data *data = (default_local_infile_data *)ptr;

  if ((count = (int)my_read(data->fd, (uchar *)buf, buf_len, MYF(0))) < 0) {
    char errbuf[MYSYS_STRERROR_SIZE];
    data->error_num = EE_READ; /* the errmsg for not entire file read */
    snprintf(data->error_msg, sizeof(data->error_msg) - 1, EE(EE_READ),
             data->filename, my_errno(),
             my_strerror(errbuf, sizeof(errbuf), my_errno()));
  }
  return count;
}


// Source: libmysql.cc
// Lines 567-579
