static int on_config_exit(h2o_configurator_t *configurator, h2o_configurator_context_t *ctx, yoml_t *node)
{
    struct throttle_resp_configurator_t *self = (void *)configurator;

    if (ctx->pathconf != NULL && self->vars->on)
        h2o_throttle_resp_register(ctx->pathconf);

    --self->vars;
    return 0;
}


// Source: throttle_resp.c
// Lines 52-61
