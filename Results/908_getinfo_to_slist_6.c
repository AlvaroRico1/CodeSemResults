// Source: curl/lib/getinfo.c
// Lines 505-507
    struct curl_slist    *to_slist;
  } ptr;

  switch(info) {
  case CURLINFO_SSL_ENGINES:
    *param_slistp = Curl_ssl_engines_list(data);
    break;
  case CURLINFO_COOKIELIST:
    *param_slistp = Curl_cookie_list(data);
    break;
  case CURLINFO_CERTINFO:
    /* Return the a pointer to the certinfo struct. Not really an slist
       pointer but we can pretend it is here */
    ptr.to_certinfo = &data->info.certs;
    *param_slistp = ptr.to_slist;
    break;
  case CURLINFO_TLS_SESSION:
  case CURLINFO_TLS_SSL_PTR:
    {
      struct curl_tlssessioninfo **tsip = (struct curl_tlssessioninfo **)
                                          param_slistp;
      struct curl_tlssessioninfo *tsi = &data->tsi;
#ifdef USE_SSL
      struct connectdata *conn = data->conn;
#endif

      *tsip = tsi;
      tsi->backend = Curl_ssl_backend();
      tsi->internals = NULL;

#ifdef USE_SSL
      if(conn && tsi->backend != CURLSSLBACKEND_NONE) {
        unsigned int i;
        for(i = 0; i < (sizeof(conn->ssl) / sizeof(conn->ssl[0])); ++i) {
          if(conn->ssl[i].use) {
            tsi->internals = Curl_ssl->get_internals(&conn->ssl[i], info);
            break;
          }
        }
      }
#endif
    }
