// Source: curl/lib/url.c
// Lines 364-365
CURLcode Curl_close(struct Curl_easy **datap)
{
  struct Curl_multi *m;
  struct Curl_easy *data;

  if(!datap || !*datap)
    return CURLE_OK;

  data = *datap;
  *datap = NULL;

  Curl_expire_clear(data); /* shut off timers */

  /* Detach connection if any is left. This should not be normal, but can be
     the case for example with CONNECT_ONLY + recv/send (test 556) */
  Curl_detach_connnection(data);
  m = data->multi;
  if(m)
    /* This handle is still part of a multi handle, take care of this first
       and detach this handle from there. */
    curl_multi_remove_handle(data->multi, data);

  if(data->multi_easy) {
    /* when curl_easy_perform() is used, it creates its own multi handle to
       use and this is the one */
    curl_multi_cleanup(data->multi_easy);
    data->multi_easy = NULL;
  }

  /* Destroy the timeout list that is held in the easy handle. It is
     /normally/ done by curl_multi_remove_handle() but this is "just in
     case" */
  Curl_llist_destroy(&data->state.timeoutlist, NULL);

  data->magic = 0; /* force a clear AFTER the possibly enforced removal from
                      the multi handle, since that function uses the magic
                      field! */

  if(data->state.rangestringalloc)
    free(data->state.range);

  /* freed here just in case DONE wasn't called */
  Curl_free_request_state(data);

  /* Close down all open SSL info and sessions */
  Curl_ssl_close_all(data);
  Curl_safefree(data->state.first_host);
  Curl_safefree(data->state.scratch);
  Curl_ssl_free_certinfo(data);

  /* Cleanup possible redirect junk */
  free(data->req.newurl);
  data->req.newurl = NULL;

  if(data->state.referer_alloc) {
    Curl_safefree(data->state.referer);
    data->state.referer_alloc = FALSE;
  }
  data->state.referer = NULL;

  up_free(data);
  Curl_safefree(data->state.buffer);
  Curl_dyn_free(&data->state.headerb);
  Curl_safefree(data->state.ulbuf);
  Curl_flush_cookies(data, TRUE);
  Curl_altsvc_save(data, data->asi, data->set.str[STRING_ALTSVC]);
  Curl_altsvc_cleanup(&data->asi);
  Curl_hsts_save(data, data->hsts, data->set.str[STRING_HSTS]);
  Curl_hsts_cleanup(&data->hsts);
#if !defined(CURL_DISABLE_HTTP) && !defined(CURL_DISABLE_CRYPTO_AUTH)
  Curl_http_auth_cleanup_digest(data);
#endif
  Curl_safefree(data->info.contenttype);
  Curl_safefree(data->info.wouldredirect);

  /* this destroys the channel and we cannot use it anymore after this */
  Curl_resolver_cleanup(data->state.async.resolver);

  Curl_http2_cleanup_dependencies(data);
  Curl_convert_close(data);

  /* No longer a dirty share, if it exists */
  if(data->share) {
    Curl_share_lock(data, CURL_LOCK_DATA_SHARE, CURL_LOCK_ACCESS_SINGLE);
    data->share->dirty--;
    Curl_share_unlock(data, CURL_LOCK_DATA_SHARE);
  }

  Curl_safefree(data->state.aptr.proxyuserpwd);
  Curl_safefree(data->state.aptr.uagent);
  Curl_safefree(data->state.aptr.userpwd);
  Curl_safefree(data->state.aptr.accept_encoding);
  Curl_safefree(data->state.aptr.te);
  Curl_safefree(data->state.aptr.rangeline);
  Curl_safefree(data->state.aptr.ref);
  Curl_safefree(data->state.aptr.host);
  Curl_safefree(data->state.aptr.cookiehost);
  Curl_safefree(data->state.aptr.rtsp_transport);
  Curl_safefree(data->state.aptr.user);
  Curl_safefree(data->state.aptr.passwd);
  Curl_safefree(data->state.aptr.proxyuser);
  Curl_safefree(data->state.aptr.proxypasswd);

#ifndef CURL_DISABLE_DOH
  if(data->req.doh) {
    Curl_dyn_free(&data->req.doh->probe[0].serverdoh);
    Curl_dyn_free(&data->req.doh->probe[1].serverdoh);
    curl_slist_free_all(data->req.doh->headers);
    Curl_safefree(data->req.doh);
  }
#endif

  /* destruct wildcard structures if it is needed */
  Curl_wildcard_dtor(&data->wildcard);
  Curl_freeset(data);
  free(data);
  return CURLE_OK;
}
