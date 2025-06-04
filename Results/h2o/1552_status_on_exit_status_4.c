int on_exit_status(h2o_configurator_t *_conf, h2o_configurator_context_t *ctx, yoml_t *node)
{
    struct st_status_configurator *c = (void *)_conf;
    c->stack--;
    if (!c->stack && c->duration_stats) {
        h2o_duration_stats_register(ctx->globalconf);
    }
    return 0;
}


// Source: status.c
// Lines 65-73
