// Source: curl/lib/multi.c
// Lines 2956-2956
static CURLMcode multi_socket(struct Curl_multi *multi,
                              bool checkall,
                              curl_socket_t s,
                              int ev_bitmask,
                              int *running_handles)
{
  CURLMcode result = CURLM_OK;
  struct Curl_easy *data = NULL;
  struct Curl_tree *t;
  struct curltime now = Curl_now();

  if(checkall) {
    /* *perform() deals with running_handles on its own */
    result = curl_multi_perform(multi, running_handles);

    /* walk through each easy handle and do the socket state change magic
       and callbacks */
    if(result != CURLM_BAD_HANDLE) {
      data = multi->easyp;
      while(data && !result) {
        result = singlesocket(multi, data);
        data = data->next;
      }
    }

    /* or should we fall-through and do the timer-based stuff? */
    return result;
  }
  if(s != CURL_SOCKET_TIMEOUT) {
    struct Curl_sh_entry *entry = sh_getentry(&multi->sockhash, s);

    if(!entry)
      /* Unmatched socket, we can't act on it but we ignore this fact.  In
         real-world tests it has been proved that libevent can in fact give
         the application actions even though the socket was just previously
         asked to get removed, so thus we better survive stray socket actions
         and just move on. */
      ;
    else {
      struct Curl_hash_iterator iter;
      struct Curl_hash_element *he;

      /* the socket can be shared by many transfers, iterate */
      Curl_hash_start_iterate(&entry->transfers, &iter);
      for(he = Curl_hash_next_element(&iter); he;
          he = Curl_hash_next_element(&iter)) {
        data = (struct Curl_easy *)he->ptr;
        DEBUGASSERT(data);
        DEBUGASSERT(data->magic == CURLEASY_MAGIC_NUMBER);

        if(data->conn && !(data->conn->handler->flags & PROTOPT_DIRLOCK))
          /* set socket event bitmask if they're not locked */
          data->conn->cselect_bits = ev_bitmask;

        Curl_expire(data, 0, EXPIRE_RUN_NOW);
      }

      /* Now we fall-through and do the timer-based stuff, since we don't want
         to force the user to have to deal with timeouts as long as at least
         one connection in fact has traffic. */

      data = NULL; /* set data to NULL again to avoid calling
                      multi_runsingle() in case there's no need to */
      now = Curl_now(); /* get a newer time since the multi_runsingle() loop
                           may have taken some time */
    }
  }
  else {
    /* Asked to run due to time-out. Clear the 'lastcall' variable to force
       Curl_update_timer() to trigger a callback to the app again even if the
       same timeout is still the one to run after this call. That handles the
       case when the application asks libcurl to run the timeout
       prematurely. */
    memset(&multi->timer_lastcall, 0, sizeof(multi->timer_lastcall));
  }

  /*
   * The loop following here will go on as long as there are expire-times left
   * to process in the splay and 'data' will be re-assigned for every expired
   * handle we deal with.
   */
  do {
    /* the first loop lap 'data' can be NULL */
    if(data) {
      SIGPIPE_VARIABLE(pipe_st);

      sigpipe_ignore(data, &pipe_st);
      result = multi_runsingle(multi, &now, data);
      sigpipe_restore(&pipe_st);

      if(CURLM_OK >= result) {
        /* get the socket(s) and check if the state has been changed since
           last */
        result = singlesocket(multi, data);
        if(result)
          return result;
      }
    }

    /* Check if there's one (more) expired timer to deal with! This function
       extracts a matching node if there is one */

    multi->timetree = Curl_splaygetbest(now, multi->timetree, &t);
    if(t) {
      data = t->payload; /* assign this for next loop */
      (void)add_next_timeout(now, multi, t->payload);
    }

  } while(t);

  *running_handles = multi->num_alive;
  return result;
}
