static int on_config_exit(h2o_configurator_t *_self, h2o_configurator_context_t *ctx, yoml_t *node)
{
    struct server_timing_configurator_t *self = (void *)_self;

    if (ctx->pathconf != NULL && self->vars->mode != SERVER_TIMING_MODE_OFF)
        h2o_server_timing_register(ctx->pathconf, self->vars->mode == SERVER_TIMING_MODE_ENFORCE);

    --self->vars;
    return 0;
}


// Source: server_timing.c
// Lines 58-67
