static int multi_getsock(struct Curl_easy *data,
                         curl_socket_t *socks)
{
  struct connectdata *conn = data->conn;
  /* The no connection case can happen when this is called from
     curl_multi_remove_handle() => singlesocket() => multi_getsock().
  */
  if(!conn)
    return 0;

  switch(data->mstate) {
  default:
    return 0;

  case MSTATE_RESOLVING:
    return Curl_resolv_getsock(data, socks);

  case MSTATE_PROTOCONNECTING:
  case MSTATE_PROTOCONNECT:
    return protocol_getsock(data, conn, socks);

  case MSTATE_DO:
  case MSTATE_DOING:
    return doing_getsock(data, conn, socks);

  case MSTATE_TUNNELING:
    return waitproxyconnect_getsock(conn, socks);

  case MSTATE_CONNECTING:
    return waitconnect_getsock(conn, socks);

  case MSTATE_DOING_MORE:
    return domore_getsock(data, conn, socks);

  case MSTATE_DID: /* since is set after DO is completed, we switch to
                        waiting for the same as the PERFORMING state */
  case MSTATE_PERFORMING:
    return Curl_single_getsock(data, conn, socks);
  }

}


// Source: multi.c
// Lines 982-1022
