// Source: curl/lib/connect.c
// Lines 1555-1559
CURLcode Curl_socket(struct Curl_easy *data,
                     const struct Curl_addrinfo *ai,
                     struct Curl_sockaddr_ex *addr,
                     curl_socket_t *sockfd)
{
  struct connectdata *conn = data->conn;
  struct Curl_sockaddr_ex dummy;

  if(!addr)
    /* if the caller doesn't want info back, use a local temp copy */
    addr = &dummy;

  /*
   * The Curl_sockaddr_ex structure is basically libcurl's external API
   * curl_sockaddr structure with enough space available to directly hold
   * any protocol-specific address structures. The variable declared here
   * will be used to pass / receive data to/from the fopensocket callback
   * if this has been set, before that, it is initialized from parameters.
   */

  addr->family = ai->ai_family;
  addr->socktype = (conn->transport == TRNSPRT_TCP) ? SOCK_STREAM : SOCK_DGRAM;
  addr->protocol = conn->transport != TRNSPRT_TCP ? IPPROTO_UDP :
    ai->ai_protocol;
  addr->addrlen = ai->ai_addrlen;

  if(addr->addrlen > sizeof(struct Curl_sockaddr_storage))
     addr->addrlen = sizeof(struct Curl_sockaddr_storage);
  memcpy(&addr->sa_addr, ai->ai_addr, addr->addrlen);

  if(data->set.fopensocket) {
   /*
    * If the opensocket callback is set, all the destination address
    * information is passed to the callback. Depending on this information the
    * callback may opt to abort the connection, this is indicated returning
    * CURL_SOCKET_BAD; otherwise it will return a not-connected socket. When
    * the callback returns a valid socket the destination address information
    * might have been changed and this 'new' address will actually be used
    * here to connect.
    */
    Curl_set_in_callback(data, true);
    *sockfd = data->set.fopensocket(data->set.opensocket_client,
                                    CURLSOCKTYPE_IPCXN,
                                    (struct curl_sockaddr *)addr);
    Curl_set_in_callback(data, false);
  }
  else
    /* opensocket callback not set, so simply create the socket now */
    *sockfd = socket(addr->family, addr->socktype, addr->protocol);

  if(*sockfd == CURL_SOCKET_BAD)
    /* no socket, no connection */
    return CURLE_COULDNT_CONNECT;

  if(conn->transport == TRNSPRT_QUIC) {
    /* QUIC sockets need to be nonblocking */
    (void)curlx_nonblock(*sockfd, TRUE);
  }

#if defined(ENABLE_IPV6) && defined(HAVE_SOCKADDR_IN6_SIN6_SCOPE_ID)
  if(conn->scope_id && (addr->family == AF_INET6)) {
    struct sockaddr_in6 * const sa6 = (void *)&addr->sa_addr;
    sa6->sin6_scope_id = conn->scope_id;
  }
#endif

#if defined(__linux__) && defined(IP_RECVERR)
  if(addr->socktype == SOCK_DGRAM) {
    int one = 1;
    switch(addr->family) {
    case AF_INET:
      (void)setsockopt(*sockfd, SOL_IP, IP_RECVERR, &one, sizeof(one));
      break;
    case AF_INET6:
      (void)setsockopt(*sockfd, SOL_IPV6, IPV6_RECVERR, &one, sizeof(one));
      break;
    }
  }
#endif

  return CURLE_OK;
}
