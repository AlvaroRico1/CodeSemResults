static bool net_should_retry(NET *net,
                             uint *retry_count MY_ATTRIBUTE((unused))) {
  bool retry;

#ifndef MYSQL_SERVER
  /*
    In the  client library, interrupted I/O operations are always retried.
    Otherwise, it's either a timeout or an unrecoverable error.
  */
  retry = vio_should_retry(net->vio);
#else
  /*
    In the server, interrupted I/O operations are retried up to a limit.
    In this scenario, pthread_kill can be used to wake up
    (interrupt) threads waiting for I/O.
  */
  retry = vio_should_retry(net->vio) && ((*retry_count)++ < net->retry_count);
#endif

  return retry;
}


// Source: net_serv.cc
// Lines 304-324
