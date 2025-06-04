static bool net_read_raw_loop(NET *net, size_t count) {
  DBUG_TRACE;
  bool eof = false;
  unsigned int retry_count = 0;
  uchar *buf = net->buff + net->where_b;

  while (count) {
    size_t recvcnt = vio_read(net->vio, buf, count);

    /* VIO_SOCKET_ERROR (-1) indicates an error. */
    if (recvcnt == VIO_SOCKET_ERROR) {
      /* A recoverable I/O error occurred? */
      if (net_should_retry(net, &retry_count))
        continue;
      else
        break;
    }
    /* Zero indicates end of file. */
    else if (!recvcnt) {
      eof = true;
      break;
    }

    count -= recvcnt;
    buf += recvcnt;
#ifdef MYSQL_SERVER
    thd_increment_bytes_received(recvcnt);
#endif
  }


// Source: net_serv.cc
// Lines 1340-1368
