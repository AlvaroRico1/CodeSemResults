static bool net_write_raw_loop(NET *net, const uchar *buf, size_t count) {
  unsigned int retry_count = 0;

  while (count) {
    size_t sentcnt = vio_write(net->vio, buf, count);

    /* VIO_SOCKET_ERROR (-1) indicates an error. */
    if (sentcnt == VIO_SOCKET_ERROR) {
      /* A recoverable I/O error occurred? */
      if (net_should_retry(net, &retry_count))
        continue;
      else
        break;
    }

    count -= sentcnt;
    buf += sentcnt;
#ifdef MYSQL_SERVER
    thd_increment_bytes_sent(sentcnt);
#endif
  }


// Source: net_serv.cc
// Lines 987-1007
