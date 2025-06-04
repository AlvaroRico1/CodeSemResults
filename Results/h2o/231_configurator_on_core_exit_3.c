static int on_core_exit(h2o_configurator_t *_self, h2o_configurator_context_t *ctx, yoml_t *node)
{
    struct st_core_configurator_t *self = (void *)_self;

    if (ctx->hostconf != NULL && ctx->pathconf == NULL) {
        /* exitting from host-level configuration */
        ctx->hostconf->http2.reprioritize_blocking_assets = self->vars->http2.reprioritize_blocking_assets;
        ctx->hostconf->http2.push_preload = self->vars->http2.push_preload;
        ctx->hostconf->http2.allow_cross_origin_push = self->vars->http2.allow_cross_origin_push;
        ctx->hostconf->http2.casper = self->vars->http2.casper;
    } else if (ctx->pathconf != NULL) {
        /* exitting from path or extension-level configuration */
        ctx->pathconf->error_log.emit_request_errors = self->vars->error_log.emit_request_errors;
    }

    --self->vars;
    return 0;
}


// Source: configurator.c
// Lines 82-99
