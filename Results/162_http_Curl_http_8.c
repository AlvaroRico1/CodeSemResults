// Source: curl/lib/http.c
// Lines 3016-3018
CURLcode Curl_http(struct Curl_easy *data, bool *done)
{
  struct connectdata *conn = data->conn;
  CURLcode result = CURLE_OK;
  struct HTTP *http;
  Curl_HttpReq httpreq;
  const char *te = ""; /* transfer-encoding */
  const char *request;
  const char *httpstring;
  struct dynbuf req;
  char *altused = NULL;
  const char *p_accept;      /* Accept: string */

  /* Always consider the DO phase done after this function call, even if there
     may be parts of the request that are not yet sent, since we can deal with
     the rest of the request in the PERFORM phase. */
  *done = TRUE;

  if(conn->transport != TRNSPRT_QUIC) {
    if(conn->httpversion < 20) { /* unless the connection is re-used and
                                    already http2 */
      switch(conn->negnpn) {
      case CURL_HTTP_VERSION_2:
        conn->httpversion = 20; /* we know we're on HTTP/2 now */

        result = Curl_http2_switched(data, NULL, 0);
        if(result)
          return result;
        break;
      case CURL_HTTP_VERSION_1_1:
        /* continue with HTTP/1.1 when explicitly requested */
        break;
      default:
        /* Check if user wants to use HTTP/2 with clear TCP*/
#ifdef USE_NGHTTP2
        if(data->state.httpwant == CURL_HTTP_VERSION_2_PRIOR_KNOWLEDGE) {
#ifndef CURL_DISABLE_PROXY
          if(conn->bits.httpproxy && !conn->bits.tunnel_proxy) {
            /* We don't support HTTP/2 proxies yet. Also it's debatable
               whether or not this setting should apply to HTTP/2 proxies. */
            infof(data, "Ignoring HTTP/2 prior knowledge due to proxy");
            break;
          }
#endif
          DEBUGF(infof(data, "HTTP/2 over clean TCP"));
          conn->httpversion = 20;

          result = Curl_http2_switched(data, NULL, 0);
          if(result)
            return result;
        }
#endif
        break;
      }
    }
    else {
      /* prepare for a http2 request */
      result = Curl_http2_setup(data, conn);
      if(result)
        return result;
    }
  }
  http = data->req.p.http;
  DEBUGASSERT(http);

  result = Curl_http_host(data, conn);
  if(result)
    return result;

  result = Curl_http_useragent(data);
  if(result)
    return result;

  Curl_http_method(data, conn, &request, &httpreq);

  /* setup the authentication headers */
  {
    char *pq = NULL;
    if(data->state.up.query) {
      pq = aprintf("%s?%s", data->state.up.path, data->state.up.query);
      if(!pq)
        return CURLE_OUT_OF_MEMORY;
    }
    result = Curl_http_output_auth(data, conn, request, httpreq,
                                   (pq ? pq : data->state.up.path), FALSE);
    free(pq);
    if(result)
      return result;
  }

  Curl_safefree(data->state.aptr.ref);
  if(data->state.referer && !Curl_checkheaders(data, "Referer")) {
    data->state.aptr.ref = aprintf("Referer: %s\r\n", data->state.referer);
    if(!data->state.aptr.ref)
      return CURLE_OUT_OF_MEMORY;
  }

  if(!Curl_checkheaders(data, "Accept-Encoding") &&
     data->set.str[STRING_ENCODING]) {
    Curl_safefree(data->state.aptr.accept_encoding);
    data->state.aptr.accept_encoding =
      aprintf("Accept-Encoding: %s\r\n", data->set.str[STRING_ENCODING]);
    if(!data->state.aptr.accept_encoding)
      return CURLE_OUT_OF_MEMORY;
  }
  else
    Curl_safefree(data->state.aptr.accept_encoding);

#ifdef HAVE_LIBZ
  /* we only consider transfer-encoding magic if libz support is built-in */
  result = Curl_transferencode(data);
  if(result)
    return result;
#endif

  result = Curl_http_body(data, conn, httpreq, &te);
  if(result)
    return result;

  p_accept = Curl_checkheaders(data, "Accept")?NULL:"Accept: */*\r\n";

  result = Curl_http_resume(data, conn, httpreq);
  if(result)
    return result;

  result = Curl_http_range(data, httpreq);
  if(result)
    return result;

  httpstring = get_http_string(data, conn);

  /* initialize a dynamic send-buffer */
  Curl_dyn_init(&req, DYN_HTTP_REQUEST);

  /* make sure the header buffer is reset - if there are leftovers from a
     previous transfer */
  Curl_dyn_reset(&data->state.headerb);

  /* add the main request stuff */
  /* GET/HEAD/POST/PUT */
  result = Curl_dyn_addf(&req, "%s ", request);
  if(!result)
    result = Curl_http_target(data, conn, &req);
  if(result) {
    Curl_dyn_free(&req);
    return result;
  }

#ifndef CURL_DISABLE_ALTSVC
  if(conn->bits.altused && !Curl_checkheaders(data, "Alt-Used")) {
    altused = aprintf("Alt-Used: %s:%d\r\n",
                      conn->conn_to_host.name, conn->conn_to_port);
    if(!altused) {
      Curl_dyn_free(&req);
      return CURLE_OUT_OF_MEMORY;
    }
  }
#endif
  result =
    Curl_dyn_addf(&req,
                  " HTTP/%s\r\n" /* HTTP version */
                  "%s" /* host */
                  "%s" /* proxyuserpwd */
                  "%s" /* userpwd */
                  "%s" /* range */
                  "%s" /* user agent */
                  "%s" /* accept */
                  "%s" /* TE: */
                  "%s" /* accept-encoding */
                  "%s" /* referer */
                  "%s" /* Proxy-Connection */
                  "%s" /* transfer-encoding */
                  "%s",/* Alt-Used */

                  httpstring,
                  (data->state.aptr.host?data->state.aptr.host:""),
                  data->state.aptr.proxyuserpwd?
                  data->state.aptr.proxyuserpwd:"",
                  data->state.aptr.userpwd?data->state.aptr.userpwd:"",
                  (data->state.use_range && data->state.aptr.rangeline)?
                  data->state.aptr.rangeline:"",
                  (data->set.str[STRING_USERAGENT] &&
                   *data->set.str[STRING_USERAGENT] &&
                   data->state.aptr.uagent)?
                  data->state.aptr.uagent:"",
                  p_accept?p_accept:"",
                  data->state.aptr.te?data->state.aptr.te:"",
                  (data->set.str[STRING_ENCODING] &&
                   *data->set.str[STRING_ENCODING] &&
                   data->state.aptr.accept_encoding)?
                  data->state.aptr.accept_encoding:"",
                  (data->state.referer && data->state.aptr.ref)?
                  data->state.aptr.ref:"" /* Referer: <data> */,
#ifndef CURL_DISABLE_PROXY
                  (conn->bits.httpproxy &&
                   !conn->bits.tunnel_proxy &&
                   !Curl_checkheaders(data, "Proxy-Connection") &&
                   !Curl_checkProxyheaders(data, conn, "Proxy-Connection"))?
                  "Proxy-Connection: Keep-Alive\r\n":"",
#else
                  "",
#endif
                  te,
                  altused ? altused : ""
      );

  /* clear userpwd and proxyuserpwd to avoid re-using old credentials
   * from re-used connections */
  Curl_safefree(data->state.aptr.userpwd);
  Curl_safefree(data->state.aptr.proxyuserpwd);
  free(altused);

  if(result) {
    Curl_dyn_free(&req);
    return result;
  }

  if(!(conn->handler->flags&PROTOPT_SSL) &&
     conn->httpversion != 20 &&
     (data->state.httpwant == CURL_HTTP_VERSION_2)) {
    /* append HTTP2 upgrade magic stuff to the HTTP request if it isn't done
       over SSL */
    result = Curl_http2_request_upgrade(&req, data);
    if(result) {
      Curl_dyn_free(&req);
      return result;
    }
  }

  result = Curl_http_cookies(data, conn, &req);
  if(!result)
    result = Curl_add_timecondition(data, &req);
  if(!result)
    result = Curl_add_custom_headers(data, FALSE, &req);

  if(!result) {
    http->postdata = NULL;  /* nothing to post at this point */
    if((httpreq == HTTPREQ_GET) ||
       (httpreq == HTTPREQ_HEAD))
      Curl_pgrsSetUploadSize(data, 0); /* nothing */

    /* bodysend takes ownership of the 'req' memory on success */
    result = Curl_http_bodysend(data, conn, &req, httpreq);
  }
  if(result) {
    Curl_dyn_free(&req);
    return result;
  }

  if((http->postsize > -1) &&
     (http->postsize <= data->req.writebytecount) &&
     (http->sending != HTTPSEND_REQUEST))
    data->req.upload_done = TRUE;

  if(data->req.writebytecount) {
    /* if a request-body has been sent off, we make sure this progress is noted
       properly */
    Curl_pgrsSetUploadCounter(data, data->req.writebytecount);
    if(Curl_pgrsUpdate(data))
      result = CURLE_ABORTED_BY_CALLBACK;

    if(!http->postsize) {
      /* already sent the entire request body, mark the "upload" as
         complete */
      infof(data, "upload completely sent off: %" CURL_FORMAT_CURL_OFF_T
            " out of %" CURL_FORMAT_CURL_OFF_T " bytes",
            data->req.writebytecount, http->postsize);
      data->req.upload_done = TRUE;
      data->req.keepon &= ~KEEP_SEND; /* we're done writing */
      data->req.exp100 = EXP100_SEND_DATA; /* already sent */
      Curl_expire_done(data, EXPIRE_100_TIMEOUT);
    }
  }

  if((conn->httpversion == 20) && data->req.upload_chunky)
    /* upload_chunky was set above to set up the request in a chunky fashion,
       but is disabled here again to avoid that the chunked encoded version is
       actually used when sending the request body over h2 */
    data->req.upload_chunky = FALSE;
  return result;
}
