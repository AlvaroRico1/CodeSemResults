static CURLcode getinfo_long(struct Curl_easy *data, CURLINFO info,
                             long *param_longp)
{
  curl_socket_t sockfd;

  union {
    unsigned long *to_ulong;
    long          *to_long;
  } lptr;


// Source: getinfo.c
// Lines 175-183
