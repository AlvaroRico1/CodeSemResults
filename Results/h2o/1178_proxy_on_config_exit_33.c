static int on_config_exit(h2o_configurator_t *_self, h2o_configurator_context_t *ctx, yoml_t *node)
{
    struct proxy_configurator_t *self = (void *)_self;

    if (ctx->pathconf == NULL && ctx->hostconf == NULL) {
        /* is global conf */
        ctx->globalconf->proxy.io_timeout = self->vars->conf.io_timeout;
        ctx->globalconf->proxy.connect_timeout = self->vars->conf.connect_timeout;
        ctx->globalconf->proxy.first_byte_timeout = self->vars->conf.first_byte_timeout;
        ctx->globalconf->proxy.keepalive_timeout = self->vars->conf.keepalive_timeout;
        ctx->globalconf->proxy.max_buffer_size = self->vars->conf.max_buffer_size;
        ctx->globalconf->proxy.http2.max_concurrent_streams = self->vars->conf.http2.max_concurrent_streams;
        ctx->globalconf->proxy.protocol_ratio.http2 = self->vars->conf.protocol_ratio.http2;
        ctx->globalconf->proxy.protocol_ratio.http3 = self->vars->conf.protocol_ratio.http3;
        h2o_socketpool_set_ssl_ctx(&ctx->globalconf->proxy.global_socketpool, self->vars->ssl_ctx);
        h2o_socketpool_set_timeout(&ctx->globalconf->proxy.global_socketpool, self->vars->conf.keepalive_timeout);
    }
    SSL_CTX_free(self->vars->ssl_ctx);

    if (self->vars->conf.headers_cmds != NULL)
        h2o_mem_release_shared(self->vars->conf.headers_cmds);

    --self->vars;
    return 0;
}


// Source: proxy.c
// Lines 582-606
