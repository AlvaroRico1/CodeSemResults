void Curl_multi_closed(struct Curl_easy *data, curl_socket_t s)
{
  if(data) {
    /* if there's still an easy handle associated with this connection */
    struct Curl_multi *multi = data->multi;
    if(multi) {
      /* this is set if this connection is part of a handle that is added to
         a multi handle, and only then this is necessary */
      struct Curl_sh_entry *entry = sh_getentry(&multi->sockhash, s);

      if(entry) {
        if(multi->socket_cb)
          multi->socket_cb(data, s, CURL_POLL_REMOVE,
                           multi->socket_userp,
                           entry->socketp);

        /* now remove it from the socket hash */
        sh_delentry(entry, &multi->sockhash, s);
      }
    }
  }


// Source: multi.c
// Lines 2870-2890
