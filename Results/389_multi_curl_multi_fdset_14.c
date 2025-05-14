// Source: curl/lib/multi.c
// Lines 1024-1031
CURLMcode curl_multi_fdset(struct Curl_multi *multi,
                           fd_set *read_fd_set, fd_set *write_fd_set,
                           fd_set *exc_fd_set, int *max_fd)
{
  /* Scan through all the easy handles to get the file descriptors set.
     Some easy handles may not have connected to the remote host yet,
     and then we must make sure that is done. */
  struct Curl_easy *data;
  int this_max_fd = -1;
  curl_socket_t sockbunch[MAX_SOCKSPEREASYHANDLE];
  int i;
  (void)exc_fd_set; /* not used */

  if(!GOOD_MULTI_HANDLE(multi))
    return CURLM_BAD_HANDLE;

  if(multi->in_callback)
    return CURLM_RECURSIVE_API_CALL;

  data = multi->easyp;
  while(data) {
    int bitmap;
#ifdef __clang_analyzer_
    /* to prevent "The left operand of '>=' is a garbage value" warnings */
    memset(sockbunch, 0, sizeof(sockbunch));
#endif
    bitmap = multi_getsock(data, sockbunch);

    for(i = 0; i< MAX_SOCKSPEREASYHANDLE; i++) {
      curl_socket_t s = CURL_SOCKET_BAD;

      if((bitmap & GETSOCK_READSOCK(i)) && VALID_SOCK((sockbunch[i]))) {
        FD_SET(sockbunch[i], read_fd_set);
        s = sockbunch[i];
      }
      if((bitmap & GETSOCK_WRITESOCK(i)) && VALID_SOCK((sockbunch[i]))) {
        FD_SET(sockbunch[i], write_fd_set);
        s = sockbunch[i];
      }
      if(s == CURL_SOCKET_BAD)
        /* this socket is unused, break out of loop */
        break;
      if((int)s > this_max_fd)
        this_max_fd = (int)s;
    }

    data = data->next; /* check next handle */
  }

  *max_fd = this_max_fd;

  return CURLM_OK;
}
