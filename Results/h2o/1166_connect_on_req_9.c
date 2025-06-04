static int on_req(h2o_handler_t *_handler, h2o_req_t *req)
{
    struct st_connect_handler_t *handler = (void *)_handler;
    h2o_iovec_t host;
    uint16_t port;
    int is_tcp;

    if (h2o_memis(req->input.method.base, req->input.method.len, H2O_STRLIT("CONNECT"))) {
        is_tcp = 1;
    } else if (h2o_memis(req->input.method.base, req->input.method.len, H2O_STRLIT("CONNECT-UDP"))) {
        is_tcp = 0;
    } else {
        return -1;
    }

    if (h2o_url_parse_hostport(req->authority.base, req->authority.len, &host, &port) == NULL || port == 0 || port == 65535) {
        h2o_send_error_400(req, "Bad Request", "Bad Request", H2O_SEND_ERROR_KEEP_HEADERS);
        return 0;
    }

    struct st_connect_generator_t *self;
    size_t sizeof_self = offsetof(struct st_connect_generator_t, tcp) + (is_tcp ? sizeof(self->tcp) : sizeof(self->udp));
    self = h2o_mem_alloc_shared(&req->pool, sizeof_self, on_generator_dispose);
    memset(self, 0, sizeof_self);
    self->super.stop = on_stop;
    self->handler = handler;
    self->src_req = req;
    self->timeout.cb = on_connect_timeout;
    if (is_tcp) {
        self->is_tcp = 1;
        self->super.proceed = tcp_on_proceed;
        h2o_buffer_init(&self->tcp.sendbuf, &h2o_socket_buffer_prototype);
    } else {
        self->super.proceed = udp_on_proceed;
        h2o_buffer_init(&self->udp.egress.buf, &h2o_socket_buffer_prototype);
        self->udp.egress.delayed = (h2o_timer_t){.cb = udp_write_stream_complete_delayed};
    }
    h2o_timer_link(get_loop(self), handler->config.connect_timeout, &self->timeout);

    /* setup write_req now, so that the protocol handler would not provide additional data until we call `proceed_req` */
    assert(req->entity.len == 0 && "the handler is incapable of accepting input via `write_req.cb` while writing req->entity");
    self->src_req->write_req.cb = is_tcp ? tcp_write : udp_write_stream;
    self->src_req->write_req.ctx = self;

    char port_str[sizeof(H2O_UINT16_LONGEST_STR)];
    int port_strlen = sprintf(port_str, "%" PRIu16, port);

    self->getaddr_req.v6 =
        h2o_hostinfo_getaddr(&self->src_req->conn->ctx->receivers.hostinfo_getaddr, host, h2o_iovec_init(port_str, port_strlen),
                             AF_INET6, is_tcp ? SOCK_STREAM : SOCK_DGRAM, is_tcp ? IPPROTO_TCP : IPPROTO_UDP,
                             AI_ADDRCONFIG | AI_NUMERICSERV, on_getaddr, self);
    self->getaddr_req.v4 =
        h2o_hostinfo_getaddr(&self->src_req->conn->ctx->receivers.hostinfo_getaddr, host, h2o_iovec_init(port_str, port_strlen),
                             AF_INET, is_tcp ? SOCK_STREAM : SOCK_DGRAM, is_tcp ? IPPROTO_TCP : IPPROTO_UDP,
                             AI_ADDRCONFIG | AI_NUMERICSERV, on_getaddr, self);

    return 0;
}


// Source: connect.c
// Lines 827-884
