static CURLcode parse_proxy(struct Curl_easy *data,
                            struct connectdata *conn, char *proxy,
                            curl_proxytype proxytype)
{
  char *portptr = NULL;
  int port = -1;
  char *proxyuser = NULL;
  char *proxypasswd = NULL;
  char *host;
  bool sockstype;
  CURLUcode uc;
  struct proxy_info *proxyinfo;
  CURLU *uhp = curl_url();
  CURLcode result = CURLE_OK;
  char *scheme = NULL;

  /* When parsing the proxy, allowing non-supported schemes since we have
     these made up ones for proxies. Guess scheme for URLs without it. */
  uc = curl_url_set(uhp, CURLUPART_URL, proxy,
                    CURLU_NON_SUPPORT_SCHEME|CURLU_GUESS_SCHEME);
  if(!uc) {
    /* parsed okay as a URL */
    uc = curl_url_get(uhp, CURLUPART_SCHEME, &scheme, 0);
    if(uc) {
      result = CURLE_OUT_OF_MEMORY;
      goto error;
    }

    if(strcasecompare("https", scheme))
      proxytype = CURLPROXY_HTTPS;
    else if(strcasecompare("socks5h", scheme))
      proxytype = CURLPROXY_SOCKS5_HOSTNAME;
    else if(strcasecompare("socks5", scheme))
      proxytype = CURLPROXY_SOCKS5;
    else if(strcasecompare("socks4a", scheme))
      proxytype = CURLPROXY_SOCKS4A;
    else if(strcasecompare("socks4", scheme) ||
            strcasecompare("socks", scheme))
      proxytype = CURLPROXY_SOCKS4;
    else if(strcasecompare("http", scheme))
      ; /* leave it as HTTP or HTTP/1.0 */
    else {
      /* Any other xxx:// reject! */
      failf(data, "Unsupported proxy scheme for \'%s\'", proxy);
      result = CURLE_COULDNT_CONNECT;
      goto error;
    }
  }
  else {
    failf(data, "Unsupported proxy syntax in \'%s\'", proxy);
    result = CURLE_COULDNT_RESOLVE_PROXY;
    goto error;
  }

#ifdef USE_SSL
  if(!(Curl_ssl->supports & SSLSUPP_HTTPS_PROXY))
#endif
    if(proxytype == CURLPROXY_HTTPS) {
      failf(data, "Unsupported proxy \'%s\', libcurl is built without the "
                  "HTTPS-proxy support.", proxy);
      result = CURLE_NOT_BUILT_IN;
      goto error;
    }

  sockstype =
    proxytype == CURLPROXY_SOCKS5_HOSTNAME ||
    proxytype == CURLPROXY_SOCKS5 ||
    proxytype == CURLPROXY_SOCKS4A ||
    proxytype == CURLPROXY_SOCKS4;

  proxyinfo = sockstype ? &conn->socks_proxy : &conn->http_proxy;
  proxyinfo->proxytype = proxytype;

  /* Is there a username and password given in this proxy url? */
  uc = curl_url_get(uhp, CURLUPART_USER, &proxyuser, CURLU_URLDECODE);
  if(uc && (uc != CURLUE_NO_USER))
    goto error;
  uc = curl_url_get(uhp, CURLUPART_PASSWORD, &proxypasswd, CURLU_URLDECODE);
  if(uc && (uc != CURLUE_NO_PASSWORD))
    goto error;

  if(proxyuser || proxypasswd) {
    Curl_safefree(proxyinfo->user);
    proxyinfo->user = proxyuser;
    result = Curl_setstropt(&data->state.aptr.proxyuser, proxyuser);
    proxyuser = NULL;
    if(result)
      goto error;
    Curl_safefree(proxyinfo->passwd);
    if(!proxypasswd) {
      proxypasswd = strdup("");
      if(!proxypasswd) {
        result = CURLE_OUT_OF_MEMORY;
        goto error;
      }
    }
    proxyinfo->passwd = proxypasswd;
    result = Curl_setstropt(&data->state.aptr.proxypasswd, proxypasswd);
    proxypasswd = NULL;
    if(result)
      goto error;
    conn->bits.proxy_user_passwd = TRUE; /* enable it */
  }

  (void)curl_url_get(uhp, CURLUPART_PORT, &portptr, 0);

  if(portptr) {
    port = (int)strtol(portptr, NULL, 10);
    free(portptr);
  }
  else {
    if(data->set.proxyport)
      /* None given in the proxy string, then get the default one if it is
         given */
      port = (int)data->set.proxyport;
    else {
      if(proxytype == CURLPROXY_HTTPS)
        port = CURL_DEFAULT_HTTPS_PROXY_PORT;
      else
        port = CURL_DEFAULT_PROXY_PORT;
    }
  }
  if(port >= 0) {
    proxyinfo->port = port;
    if(conn->port < 0 || sockstype || !conn->socks_proxy.host.rawalloc)
      conn->port = port;
  }

  /* now, clone the proxy host name */
  uc = curl_url_get(uhp, CURLUPART_HOST, &host, CURLU_URLDECODE);
  if(uc) {
    result = CURLE_OUT_OF_MEMORY;
    goto error;
  }
  Curl_safefree(proxyinfo->host.rawalloc);
  proxyinfo->host.rawalloc = host;
  if(host[0] == '[') {
    /* this is a numerical IPv6, strip off the brackets */
    size_t len = strlen(host);
    host[len-1] = 0; /* clear the trailing bracket */
    host++;
    zonefrom_url(uhp, data, conn);
  }
  proxyinfo->host.name = host;

  error:
  free(proxyuser);
  free(proxypasswd);
  free(scheme);
  curl_url_cleanup(uhp);
  return result;
}


// Source: url.c
// Lines 2367-2518
