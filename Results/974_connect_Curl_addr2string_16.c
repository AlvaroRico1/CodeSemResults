// Source: curl/lib/connect.c
// Lines 625-633
bool Curl_addr2string(struct sockaddr *sa, curl_socklen_t salen,
                      char *addr, int *port)
{
  struct sockaddr_in *si = NULL;
#ifdef ENABLE_IPV6
  struct sockaddr_in6 *si6 = NULL;
#endif
#if defined(HAVE_SYS_UN_H) && defined(AF_UNIX)
  struct sockaddr_un *su = NULL;
#else
  (void)salen;
#endif

  switch(sa->sa_family) {
    case AF_INET:
      si = (struct sockaddr_in *)(void *) sa;
      if(Curl_inet_ntop(sa->sa_family, &si->sin_addr,
                        addr, MAX_IPADR_LEN)) {
        unsigned short us_port = ntohs(si->sin_port);
        *port = us_port;
        return TRUE;
      }
      break;
#ifdef ENABLE_IPV6
    case AF_INET6:
      si6 = (struct sockaddr_in6 *)(void *) sa;
      if(Curl_inet_ntop(sa->sa_family, &si6->sin6_addr,
                        addr, MAX_IPADR_LEN)) {
        unsigned short us_port = ntohs(si6->sin6_port);
        *port = us_port;
        return TRUE;
      }
      break;
#endif
#if defined(HAVE_SYS_UN_H) && defined(AF_UNIX)
    case AF_UNIX:
      if(salen > (curl_socklen_t)sizeof(CURL_SA_FAMILY_T)) {
        su = (struct sockaddr_un*)sa;
        msnprintf(addr, MAX_IPADR_LEN, "%s", su->sun_path);
      }
      else
        addr[0] = 0; /* socket with no name */
      *port = 0;
      return TRUE;
#endif
    default:
      break;
  }

  addr[0] = '\0';
  *port = 0;
  errno = EAFNOSUPPORT;
  return FALSE;
}
