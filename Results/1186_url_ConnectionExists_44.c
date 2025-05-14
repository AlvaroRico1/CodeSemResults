// Source: curl/lib/url.c
// Lines 1306-1307
ConnectionExists(struct Curl_easy *data,
                 struct connectdata *needle,
                 struct connectdata **usethis,
                 bool *force_reuse,
                 bool *waitpipe)
{
  struct connectdata *check;
  struct connectdata *chosen = 0;
  bool foundPendingCandidate = FALSE;
  bool canmultiplex = IsMultiplexingPossible(data, needle);
  struct connectbundle *bundle;
  const char *hostbundle;

#ifdef USE_NTLM
  bool wantNTLMhttp = ((data->state.authhost.want &
                        (CURLAUTH_NTLM | CURLAUTH_NTLM_WB)) &&
                       (needle->handler->protocol & PROTO_FAMILY_HTTP));
#ifndef CURL_DISABLE_PROXY
  bool wantProxyNTLMhttp = (needle->bits.proxy_user_passwd &&
                            ((data->state.authproxy.want &
                              (CURLAUTH_NTLM | CURLAUTH_NTLM_WB)) &&
                             (needle->handler->protocol & PROTO_FAMILY_HTTP)));
#else
  bool wantProxyNTLMhttp = FALSE;
#endif
#endif

  *force_reuse = FALSE;
  *waitpipe = FALSE;

  /* Look up the bundle with all the connections to this particular host.
     Locks the connection cache, beware of early returns! */
  bundle = Curl_conncache_find_bundle(data, needle, data->state.conn_cache,
                                      &hostbundle);
  if(bundle) {
    /* Max pipe length is zero (unlimited) for multiplexed connections */
    struct Curl_llist_element *curr;

    infof(data, "Found bundle for host %s: %p [%s]",
          hostbundle, (void *)bundle, (bundle->multiuse == BUNDLE_MULTIPLEX ?
                                       "can multiplex" : "serially"));

    /* We can't multiplex if we don't know anything about the server */
    if(canmultiplex) {
      if(bundle->multiuse == BUNDLE_UNKNOWN) {
        if(data->set.pipewait) {
          infof(data, "Server doesn't support multiplex yet, wait");
          *waitpipe = TRUE;
          CONNCACHE_UNLOCK(data);
          return FALSE; /* no re-use */
        }

        infof(data, "Server doesn't support multiplex (yet)");
        canmultiplex = FALSE;
      }
      if((bundle->multiuse == BUNDLE_MULTIPLEX) &&
         !Curl_multiplex_wanted(data->multi)) {
        infof(data, "Could multiplex, but not asked to!");
        canmultiplex = FALSE;
      }
      if(bundle->multiuse == BUNDLE_NO_MULTIUSE) {
        infof(data, "Can not multiplex, even if we wanted to!");
        canmultiplex = FALSE;
      }
    }

    curr = bundle->conn_list.head;
    while(curr) {
      bool match = FALSE;
      size_t multiplexed = 0;

      /*
       * Note that if we use a HTTP proxy in normal mode (no tunneling), we
       * check connections to that proxy and not to the actual remote server.
       */
      check = curr->ptr;
      curr = curr->next;

      if(check->bits.connect_only || check->bits.close)
        /* connect-only or to-be-closed connections will not be reused */
        continue;

      if(extract_if_dead(check, data)) {
        /* disconnect it */
        (void)Curl_disconnect(data, check, TRUE);
        continue;
      }

      if(data->set.ipver != CURL_IPRESOLVE_WHATEVER
          && data->set.ipver != check->ip_version) {
        /* skip because the connection is not via the requested IP version */
        continue;
      }

      if(bundle->multiuse == BUNDLE_MULTIPLEX)
        multiplexed = CONN_INUSE(check);

      if(!canmultiplex) {
        if(multiplexed) {
          /* can only happen within multi handles, and means that another easy
             handle is using this connection */
          continue;
        }

        if(Curl_resolver_asynch()) {
          /* primary_ip[0] is NUL only if the resolving of the name hasn't
             completed yet and until then we don't re-use this connection */
          if(!check->primary_ip[0]) {
            infof(data,
                  "Connection #%ld is still name resolving, can't reuse",
                  check->connection_id);
            continue;
          }
        }

        if(check->sock[FIRSTSOCKET] == CURL_SOCKET_BAD) {
          foundPendingCandidate = TRUE;
          /* Don't pick a connection that hasn't connected yet */
          infof(data, "Connection #%ld isn't open enough, can't reuse",
                check->connection_id);
          continue;
        }
      }

#ifdef USE_UNIX_SOCKETS
      if(needle->unix_domain_socket) {
        if(!check->unix_domain_socket)
          continue;
        if(strcmp(needle->unix_domain_socket, check->unix_domain_socket))
          continue;
        if(needle->bits.abstract_unix_socket !=
           check->bits.abstract_unix_socket)
          continue;
      }
      else if(check->unix_domain_socket)
        continue;
#endif

      if((needle->handler->flags&PROTOPT_SSL) !=
         (check->handler->flags&PROTOPT_SSL))
        /* don't do mixed SSL and non-SSL connections */
        if(get_protocol_family(check->handler) !=
           needle->handler->protocol || !check->bits.tls_upgraded)
          /* except protocols that have been upgraded via TLS */
          continue;

#ifndef CURL_DISABLE_PROXY
      if(needle->bits.httpproxy != check->bits.httpproxy ||
         needle->bits.socksproxy != check->bits.socksproxy)
        continue;

      if(needle->bits.socksproxy &&
        !socks_proxy_info_matches(&needle->socks_proxy,
                                  &check->socks_proxy))
        continue;
#endif
      if(needle->bits.conn_to_host != check->bits.conn_to_host)
        /* don't mix connections that use the "connect to host" feature and
         * connections that don't use this feature */
        continue;

      if(needle->bits.conn_to_port != check->bits.conn_to_port)
        /* don't mix connections that use the "connect to port" feature and
         * connections that don't use this feature */
        continue;

#ifndef CURL_DISABLE_PROXY
      if(needle->bits.httpproxy) {
        if(!proxy_info_matches(&needle->http_proxy, &check->http_proxy))
          continue;

        if(needle->bits.tunnel_proxy != check->bits.tunnel_proxy)
          continue;

        if(needle->http_proxy.proxytype == CURLPROXY_HTTPS) {
          /* use https proxy */
          if(needle->handler->flags&PROTOPT_SSL) {
            /* use double layer ssl */
            if(!Curl_ssl_config_matches(&needle->proxy_ssl_config,
                                        &check->proxy_ssl_config))
              continue;
            if(check->proxy_ssl[FIRSTSOCKET].state != ssl_connection_complete)
              continue;
          }
          else {
            if(!Curl_ssl_config_matches(&needle->ssl_config,
                                        &check->ssl_config))
              continue;
            if(check->ssl[FIRSTSOCKET].state != ssl_connection_complete)
              continue;
          }
        }
      }
#endif

      if(!canmultiplex && CONN_INUSE(check))
        /* this request can't be multiplexed but the checked connection is
           already in use so we skip it */
        continue;

      if(CONN_INUSE(check)) {
        /* Subject for multiplex use if 'checks' belongs to the same multi
           handle as 'data' is. */
        struct Curl_llist_element *e = check->easyq.head;
        struct Curl_easy *entry = e->ptr;
        if(entry->multi != data->multi)
          continue;
      }
