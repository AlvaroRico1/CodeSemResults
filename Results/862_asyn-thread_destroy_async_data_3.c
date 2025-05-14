// Source: curl/lib/asyn-thread.c
// Lines 379-382
static void destroy_async_data(struct Curl_async *async)
{
  if(async->tdata) {
    struct thread_data *td = async->tdata;
    int done;
#ifndef CURL_DISABLE_SOCKETPAIR
    curl_socket_t sock_rd = td->tsd.sock_pair[0];
    struct Curl_easy *data = td->tsd.data;
#endif

    /*
     * if the thread is still blocking in the resolve syscall, detach it and
     * let the thread do the cleanup...
     */
    Curl_mutex_acquire(td->tsd.mtx);
    done = td->tsd.done;
    td->tsd.done = 1;
    Curl_mutex_release(td->tsd.mtx);

    if(!done) {
      Curl_thread_destroy(td->thread_hnd);
    }
    else {
      if(td->thread_hnd != curl_thread_t_null)
        Curl_thread_join(&td->thread_hnd);

      destroy_thread_sync_data(&td->tsd);

      free(async->tdata);
    }
#ifndef CURL_DISABLE_SOCKETPAIR
    /*
     * ensure CURLMOPT_SOCKETFUNCTION fires CURL_POLL_REMOVE
     * before the FD is invalidated to avoid EBADF on EPOLL_CTL_DEL
     */
    Curl_multi_closed(data, sock_rd);
    sclose(sock_rd);
#endif
  }
