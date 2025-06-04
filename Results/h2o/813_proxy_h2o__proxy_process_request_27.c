void h2o__proxy_process_request(h2o_req_t *req)
{
    h2o_req_overrides_t *overrides = req->overrides;
    h2o_httpclient_ctx_t *client_ctx = get_client_ctx(req);
    h2o_url_t target_buf, *target = &target_buf;

    h2o_httpclient_connection_pool_t *connpool = &req->conn->ctx->proxy.connpool;
    if (overrides != NULL && overrides->connpool != NULL) {
        connpool = overrides->connpool;
        if (!overrides->proxy_preserve_host)
            target = NULL;
    }
    if (target == &target_buf)
        h2o_url_init(&target_buf, req->scheme, req->authority, h2o_iovec_init(H2O_STRLIT("/")));

    const char *upgrade_to = NULL;
    if (req->is_tunnel_req) {
        if (req->upgrade.base != NULL) {
            /* upgrade requests (e.g. websocket) are either tunnelled or converted to a normal request (by omitting the Upgrade
             * header field)  depending on the configuration */
            if (client_ctx->tunnel_enabled)
                upgrade_to = h2o_strdup(&req->pool, req->upgrade.base, req->upgrade.len).base;
        } else {
            /* CONNECT request; process as a CONNECT upgrade or reject */
            if (client_ctx->tunnel_enabled) {
                upgrade_to = h2o_httpclient_upgrade_to_connect;
            } else {
                h2o_send_error_405(req, "Method Not Allowed", "refusing CONNECT", H2O_SEND_ERROR_HTTP1_CLOSE_CONNECTION);
                return;
            }
        }
    }
    struct rp_generator_t *self = proxy_send_prepare(req);

    /*
      When the PROXY protocol is being used (i.e. when overrides->use_proxy_protocol is set), the client needs to establish a new
     connection even when there is a pooled connection to the peer, since the header (as defined in
     https://www.haproxy.org/download/1.8/doc/proxy-protocol.txt) needs to be sent at the beginning of the connection.

     However, currently h2o_http1client_connect doesn't provide an interface to enforce estabilishing a new connection. In other
     words, there is a chance that we would use a pool connection here.

     OTOH, the probability of seeing such issue is rare; it would only happen if the same destination identified by its host:port is
     accessed in both ways (i.e. in one path with use_proxy_protocol set and in the other path without).

     So I leave this as it is for the time being.
     */
    h2o_httpclient_connect(&self->client, &req->pool, self, client_ctx, connpool, target, upgrade_to, on_connect);
}


// Source: proxy.c
// Lines 692-740
