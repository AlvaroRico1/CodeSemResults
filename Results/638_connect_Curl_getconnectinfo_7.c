// Source: curl/lib/connect.c
// Lines 1451-1461
curl_socket_t Curl_getconnectinfo(struct Curl_easy *data,
                                  struct connectdata **connp)
{
  DEBUGASSERT(data);

  /* this works for an easy handle:
   * - that has been used for curl_easy_perform()
   * - that is associated with a multi handle, and whose connection
   *   was detached with CURLOPT_CONNECT_ONLY
   */
  if((data->state.lastconnect_id != -1) && (data->multi_easy || data->multi)) {
    struct connectdata *c;
    struct connfind find;
    find.id_tofind = data->state.lastconnect_id;
    find.found = NULL;

    Curl_conncache_foreach(data, data->multi_easy?
                           &data->multi_easy->conn_cache:
                           &data->multi->conn_cache, &find, conn_is_conn);

    if(!find.found) {
      data->state.lastconnect_id = -1;
      return CURL_SOCKET_BAD;
    }

    c = find.found;
    if(connp)
      /* only store this if the caller cares for it */
      *connp = c;
    return c->sock[FIRSTSOCKET];
  }
