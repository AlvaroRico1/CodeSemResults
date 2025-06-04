static int on_config_exit(h2o_configurator_t *_self, h2o_configurator_context_t *ctx, yoml_t *node)
{
    struct reproxy_configurator_t *self = (void *)_self;

    if (ctx->pathconf != NULL && self->vars->enabled != 0)
        h2o_reproxy_register(ctx->pathconf);

    --self->vars;
    return 0;
}


// Source: reproxy.c
// Lines 56-65
