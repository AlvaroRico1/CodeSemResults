CURLcode Curl_add_custom_headers(struct Curl_easy *data,
                                 bool is_connect,
#ifndef USE_HYPER
                                 struct dynbuf *req
#else
                                 void *req
#endif
  )
{
  struct connectdata *conn = data->conn;
  char *ptr;
  struct curl_slist *h[2];
  struct curl_slist *headers;
  int numlists = 1; /* by default */
  int i;

#ifndef CURL_DISABLE_PROXY
  enum proxy_use proxy;

  if(is_connect)
    proxy = HEADER_CONNECT;
  else
    proxy = conn->bits.httpproxy && !conn->bits.tunnel_proxy?
      HEADER_PROXY:HEADER_SERVER;

  switch(proxy) {
  case HEADER_SERVER:
    h[0] = data->set.headers;
    break;
  case HEADER_PROXY:
    h[0] = data->set.headers;
    if(data->set.sep_headers) {
      h[1] = data->set.proxyheaders;
      numlists++;
    }
    break;
  case HEADER_CONNECT:
    if(data->set.sep_headers)
      h[0] = data->set.proxyheaders;
    else
      h[0] = data->set.headers;
    break;
  }
#else
  (void)is_connect;
  h[0] = data->set.headers;
#endif

  /* loop through one or two lists */
  for(i = 0; i < numlists; i++) {
    headers = h[i];

    while(headers) {
      char *semicolonp = NULL;
      ptr = strchr(headers->data, ':');
      if(!ptr) {
        char *optr;
        /* no colon, semicolon? */
        ptr = strchr(headers->data, ';');
        if(ptr) {
          optr = ptr;
          ptr++; /* pass the semicolon */
          while(*ptr && ISSPACE(*ptr))
            ptr++;

          if(*ptr) {
            /* this may be used for something else in the future */
            optr = NULL;
          }
          else {
            if(*(--ptr) == ';') {
              /* copy the source */
              semicolonp = strdup(headers->data);
              if(!semicolonp) {
#ifndef USE_HYPER
                Curl_dyn_free(req);
#endif
                return CURLE_OUT_OF_MEMORY;
              }
              /* put a colon where the semicolon is */
              semicolonp[ptr - headers->data] = ':';
              /* point at the colon */
              optr = &semicolonp [ptr - headers->data];
            }
          }
          ptr = optr;
        }
      }


// Source: http.c
// Lines 1785-1872
