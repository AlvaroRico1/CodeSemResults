// Source: curl/lib/ftp.c
// Lines 943-967
static CURLcode ftp_state_use_port(struct Curl_easy *data,
                                   ftpport fcmd) /* start with this */
{
  CURLcode result = CURLE_OK;
  struct connectdata *conn = data->conn;
  struct ftp_conn *ftpc = &conn->proto.ftpc;
  curl_socket_t portsock = CURL_SOCKET_BAD;
  char myhost[MAX_IPADR_LEN + 1] = "";

  struct Curl_sockaddr_storage ss;
  struct Curl_addrinfo *res, *ai;
  curl_socklen_t sslen;
  char hbuf[NI_MAXHOST];
  struct sockaddr *sa = (struct sockaddr *)&ss;
  struct sockaddr_in * const sa4 = (void *)sa;
#ifdef ENABLE_IPV6
  struct sockaddr_in6 * const sa6 = (void *)sa;
#endif
  static const char mode[][5] = { "EPRT", "PORT" };
  enum resolve_t rc;
  int error;
  char *host = NULL;
  char *string_ftpport = data->set.str[STRING_FTPPORT];
  struct Curl_dns_entry *h = NULL;
  unsigned short port_min = 0;
  unsigned short port_max = 0;
  unsigned short port;
  bool possibly_non_local = TRUE;
  char buffer[STRERROR_LEN];
  char *addr = NULL;

  /* Step 1, figure out what is requested,
   * accepted format :
   * (ipv4|ipv6|domain|interface)?(:port(-range)?)?
   */

  if(data->set.str[STRING_FTPPORT] &&
     (strlen(data->set.str[STRING_FTPPORT]) > 1)) {

#ifdef ENABLE_IPV6
    size_t addrlen = INET6_ADDRSTRLEN > strlen(string_ftpport) ?
      INET6_ADDRSTRLEN : strlen(string_ftpport);
#else
    size_t addrlen = INET_ADDRSTRLEN > strlen(string_ftpport) ?
      INET_ADDRSTRLEN : strlen(string_ftpport);
#endif
    char *ip_start = string_ftpport;
    char *ip_end = NULL;
    char *port_start = NULL;
    char *port_sep = NULL;

    addr = calloc(addrlen + 1, 1);
    if(!addr)
      return CURLE_OUT_OF_MEMORY;

#ifdef ENABLE_IPV6
    if(*string_ftpport == '[') {
      /* [ipv6]:port(-range) */
      ip_start = string_ftpport + 1;
      ip_end = strchr(string_ftpport, ']');
      if(ip_end)
        strncpy(addr, ip_start, ip_end - ip_start);
    }
    else
#endif
      if(*string_ftpport == ':') {
        /* :port */
        ip_end = string_ftpport;
      }
      else {
        ip_end = strchr(string_ftpport, ':');
        if(ip_end) {
          /* either ipv6 or (ipv4|domain|interface):port(-range) */
#ifdef ENABLE_IPV6
          if(Curl_inet_pton(AF_INET6, string_ftpport, sa6) == 1) {
            /* ipv6 */
            port_min = port_max = 0;
            strcpy(addr, string_ftpport);
            ip_end = NULL; /* this got no port ! */
          }
          else
#endif
            /* (ipv4|domain|interface):port(-range) */
            strncpy(addr, string_ftpport, ip_end - ip_start);
        }
        else
          /* ipv4|interface */
          strcpy(addr, string_ftpport);
      }

    /* parse the port */
    if(ip_end != NULL) {
      port_start = strchr(ip_end, ':');
      if(port_start) {
        port_min = curlx_ultous(strtoul(port_start + 1, NULL, 10));
        port_sep = strchr(port_start, '-');
        if(port_sep) {
          port_max = curlx_ultous(strtoul(port_sep + 1, NULL, 10));
        }
        else
          port_max = port_min;
      }
    }

    /* correct errors like:
     *  :1234-1230
     *  :-4711,  in this case port_min is (unsigned)-1,
     *           therefore port_min > port_max for all cases
     *           but port_max = (unsigned)-1
     */
    if(port_min > port_max)
      port_min = port_max = 0;


    if(*addr != '\0') {
      /* attempt to get the address of the given interface name */
      switch(Curl_if2ip(conn->ip_addr->ai_family,
                        Curl_ipv6_scope(conn->ip_addr->ai_addr),
                        conn->scope_id, addr, hbuf, sizeof(hbuf))) {
        case IF2IP_NOT_FOUND:
          /* not an interface, use the given string as host name instead */
          host = addr;
          break;
        case IF2IP_AF_NOT_SUPPORTED:
          return CURLE_FTP_PORT_FAILED;
        case IF2IP_FOUND:
          host = hbuf; /* use the hbuf for host name */
      }
    }
    else
      /* there was only a port(-range) given, default the host */
      host = NULL;
  } /* data->set.ftpport */
