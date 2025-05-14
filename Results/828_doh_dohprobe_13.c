// Source: curl/lib/doh.c
// Lines 219-224
static CURLcode dohprobe(struct Curl_easy *data,
                         struct dnsprobe *p, DNStype dnstype,
                         const char *host,
                         const char *url, CURLM *multi,
                         struct curl_slist *headers)
{
  struct Curl_easy *doh = NULL;
  char *nurl = NULL;
  CURLcode result = CURLE_OK;
  timediff_t timeout_ms;
  DOHcode d = doh_encode(host, dnstype, p->dohbuffer, sizeof(p->dohbuffer),
                         &p->dohlen);
  if(d) {
    failf(data, "Failed to encode DoH packet [%d]", d);
    return CURLE_OUT_OF_MEMORY;
  }

  p->dnstype = dnstype;
  Curl_dyn_init(&p->serverdoh, DYN_DOH_RESPONSE);

  /* Note: this is code for sending the DoH request with GET but there's still
     no logic that actually enables this. We should either add that ability or
     yank out the GET code. Discuss! */
  if(data->set.doh_get) {
    char *b64;
    size_t b64len;
    result = Curl_base64url_encode(data, (char *)p->dohbuffer, p->dohlen,
                                   &b64, &b64len);
    if(result)
      goto error;
    nurl = aprintf("%s?dns=%s", url, b64);
    free(b64);
    if(!nurl) {
      result = CURLE_OUT_OF_MEMORY;
      goto error;
    }
    url = nurl;
  }

  timeout_ms = Curl_timeleft(data, NULL, TRUE);
  if(timeout_ms <= 0) {
    result = CURLE_OPERATION_TIMEDOUT;
    goto error;
  }
  /* Curl_open() is the internal version of curl_easy_init() */
  result = Curl_open(&doh);
  if(!result) {
    /* pass in the struct pointer via a local variable to please coverity and
       the gcc typecheck helpers */
    struct dynbuf *resp = &p->serverdoh;
    ERROR_CHECK_SETOPT(CURLOPT_URL, url);
    ERROR_CHECK_SETOPT(CURLOPT_WRITEFUNCTION, doh_write_cb);
    ERROR_CHECK_SETOPT(CURLOPT_WRITEDATA, resp);
    if(!data->set.doh_get) {
      ERROR_CHECK_SETOPT(CURLOPT_POSTFIELDS, p->dohbuffer);
      ERROR_CHECK_SETOPT(CURLOPT_POSTFIELDSIZE, (long)p->dohlen);
    }
    ERROR_CHECK_SETOPT(CURLOPT_HTTPHEADER, headers);
#ifdef USE_NGHTTP2
    ERROR_CHECK_SETOPT(CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2TLS);
#endif
#ifndef CURLDEBUG
    /* enforce HTTPS if not debug */
    ERROR_CHECK_SETOPT(CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);
#else
    /* in debug mode, also allow http */
    ERROR_CHECK_SETOPT(CURLOPT_PROTOCOLS, CURLPROTO_HTTP|CURLPROTO_HTTPS);
#endif
    ERROR_CHECK_SETOPT(CURLOPT_TIMEOUT_MS, (long)timeout_ms);
    ERROR_CHECK_SETOPT(CURLOPT_SHARE, data->share);
    if(data->set.err && data->set.err != stderr)
      ERROR_CHECK_SETOPT(CURLOPT_STDERR, data->set.err);
    if(data->set.verbose)
      ERROR_CHECK_SETOPT(CURLOPT_VERBOSE, 1L);
    if(data->set.no_signal)
      ERROR_CHECK_SETOPT(CURLOPT_NOSIGNAL, 1L);

    ERROR_CHECK_SETOPT(CURLOPT_SSL_VERIFYHOST,
      data->set.doh_verifyhost ? 2L : 0L);
    ERROR_CHECK_SETOPT(CURLOPT_SSL_VERIFYPEER,
      data->set.doh_verifypeer ? 1L : 0L);
    ERROR_CHECK_SETOPT(CURLOPT_SSL_VERIFYSTATUS,
      data->set.doh_verifystatus ? 1L : 0L);

    /* Inherit *some* SSL options from the user's transfer. This is a
       best-guess as to which options are needed for compatibility. #3661

       Note DoH does not inherit the user's proxy server so proxy SSL settings
       have no effect and are not inherited. If that changes then two new
       options should be added to check doh proxy insecure separately,
       CURLOPT_DOH_PROXY_SSL_VERIFYHOST and CURLOPT_DOH_PROXY_SSL_VERIFYPEER.
       */
    if(data->set.ssl.falsestart)
      ERROR_CHECK_SETOPT(CURLOPT_SSL_FALSESTART, 1L);
    if(data->set.str[STRING_SSL_CAFILE]) {
      ERROR_CHECK_SETOPT(CURLOPT_CAINFO,
                         data->set.str[STRING_SSL_CAFILE]);
    }
    if(data->set.blobs[BLOB_CAINFO]) {
      ERROR_CHECK_SETOPT(CURLOPT_CAINFO_BLOB,
                         data->set.blobs[BLOB_CAINFO]);
    }
    if(data->set.str[STRING_SSL_CAPATH]) {
      ERROR_CHECK_SETOPT(CURLOPT_CAPATH,
                         data->set.str[STRING_SSL_CAPATH]);
    }
    if(data->set.str[STRING_SSL_CRLFILE]) {
      ERROR_CHECK_SETOPT(CURLOPT_CRLFILE,
                         data->set.str[STRING_SSL_CRLFILE]);
    }
    if(data->set.ssl.certinfo)
      ERROR_CHECK_SETOPT(CURLOPT_CERTINFO, 1L);
    if(data->set.str[STRING_SSL_RANDOM_FILE]) {
      ERROR_CHECK_SETOPT(CURLOPT_RANDOM_FILE,
                         data->set.str[STRING_SSL_RANDOM_FILE]);
    }
    if(data->set.str[STRING_SSL_EGDSOCKET]) {
      ERROR_CHECK_SETOPT(CURLOPT_EGDSOCKET,
                         data->set.str[STRING_SSL_EGDSOCKET]);
    }
    if(data->set.ssl.fsslctx)
      ERROR_CHECK_SETOPT(CURLOPT_SSL_CTX_FUNCTION, data->set.ssl.fsslctx);
    if(data->set.ssl.fsslctxp)
      ERROR_CHECK_SETOPT(CURLOPT_SSL_CTX_DATA, data->set.ssl.fsslctxp);
    if(data->set.str[STRING_SSL_EC_CURVES]) {
      ERROR_CHECK_SETOPT(CURLOPT_SSL_EC_CURVES,
                         data->set.str[STRING_SSL_EC_CURVES]);
    }

    {
      long mask =
        (data->set.ssl.enable_beast ?
         CURLSSLOPT_ALLOW_BEAST : 0) |
        (data->set.ssl.no_revoke ?
         CURLSSLOPT_NO_REVOKE : 0) |
        (data->set.ssl.no_partialchain ?
         CURLSSLOPT_NO_PARTIALCHAIN : 0) |
        (data->set.ssl.revoke_best_effort ?
         CURLSSLOPT_REVOKE_BEST_EFFORT : 0) |
        (data->set.ssl.native_ca_store ?
         CURLSSLOPT_NATIVE_CA : 0) |
        (data->set.ssl.auto_client_cert ?
         CURLSSLOPT_AUTO_CLIENT_CERT : 0);

      (void)curl_easy_setopt(doh, CURLOPT_SSL_OPTIONS, mask);
    }

    doh->set.fmultidone = doh_done;
    doh->set.dohfor = data; /* identify for which transfer this is done */
    p->easy = doh;

    /* DoH private_data must be null because the user must have a way to
       distinguish their transfer's handle from DoH handles in user
       callbacks (ie SSL CTX callback). */
    DEBUGASSERT(!doh->set.private_data);

    if(curl_multi_add_handle(multi, doh))
      goto error;
  }
  else
    goto error;
  free(nurl);
  return CURLE_OK;

  error:
  free(nurl);
  Curl_close(&doh);
  return result;
}
