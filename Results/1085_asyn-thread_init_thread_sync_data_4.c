// Source: curl/lib/asyn-thread.c
// Lines 217-222
int init_thread_sync_data(struct thread_data *td,
                           const char *hostname,
                           int port,
                           const struct addrinfo *hints)
{
  struct thread_sync_data *tsd = &td->tsd;

  memset(tsd, 0, sizeof(*tsd));

  tsd->td = td;
  tsd->port = port;
  /* Treat the request as done until the thread actually starts so any early
   * cleanup gets done properly.
   */
  tsd->done = 1;
#ifdef HAVE_GETADDRINFO
  DEBUGASSERT(hints);
  tsd->hints = *hints;
#else
  (void) hints;
#endif

  tsd->mtx = malloc(sizeof(curl_mutex_t));
  if(!tsd->mtx)
    goto err_exit;

  Curl_mutex_init(tsd->mtx);

#ifndef CURL_DISABLE_SOCKETPAIR
  /* create socket pair, avoid AF_LOCAL since it doesn't build on Solaris */
  if(Curl_socketpair(AF_UNIX, SOCK_STREAM, 0, &tsd->sock_pair[0]) < 0) {
    tsd->sock_pair[0] = CURL_SOCKET_BAD;
    tsd->sock_pair[1] = CURL_SOCKET_BAD;
    goto err_exit;
  }
#endif
  tsd->sock_error = CURL_ASYNC_SUCCESS;

  /* Copying hostname string because original can be destroyed by parent
   * thread during gethostbyname execution.
   */
  tsd->hostname = strdup(hostname);
  if(!tsd->hostname)
    goto err_exit;

  return 1;

 err_exit:
  /* Memory allocation failed */
  destroy_thread_sync_data(tsd);
  return 0;
}
