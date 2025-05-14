// Source: curl/lib/asyn-thread.c
// Lines 293-295
static unsigned int CURL_STDCALL getaddrinfo_thread(void *arg)
{
  struct thread_sync_data *tsd = (struct thread_sync_data *)arg;
  struct thread_data *td = tsd->td;
  char service[12];
  int rc;
#ifndef CURL_DISABLE_SOCKETPAIR
  char buf[1];
#endif

  msnprintf(service, sizeof(service), "%d", tsd->port);

  rc = Curl_getaddrinfo_ex(tsd->hostname, service, &tsd->hints, &tsd->res);

  if(rc) {
    tsd->sock_error = SOCKERRNO?SOCKERRNO:rc;
    if(tsd->sock_error == 0)
      tsd->sock_error = RESOLVER_ENOMEM;
  }
  else {
    Curl_addrinfo_set_port(tsd->res, tsd->port);
  }

  Curl_mutex_acquire(tsd->mtx);
  if(tsd->done) {
    /* too late, gotta clean up the mess */
    Curl_mutex_release(tsd->mtx);
    destroy_thread_sync_data(tsd);
    free(td);
  }
  else {
#ifndef CURL_DISABLE_SOCKETPAIR
    if(tsd->sock_pair[1] != CURL_SOCKET_BAD) {
      /* DNS has been resolved, signal client task */
      buf[0] = 1;
      if(swrite(tsd->sock_pair[1],  buf, sizeof(buf)) < 0) {
        /* update sock_erro to errno */
        tsd->sock_error = SOCKERRNO;
      }
    }
#endif
    tsd->done = 1;
    Curl_mutex_release(tsd->mtx);
  }

  return 0;
}
