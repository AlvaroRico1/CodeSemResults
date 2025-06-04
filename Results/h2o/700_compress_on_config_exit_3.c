static int on_config_exit(h2o_configurator_t *configurator, h2o_configurator_context_t *ctx, yoml_t *node)
{
    struct compress_configurator_t *self = (void *)configurator;

    if (ctx->pathconf != NULL && (self->vars->gzip.quality != -1 || self->vars->brotli.quality != -1))
        h2o_compress_register(ctx->pathconf, self->vars);

    --self->vars;
    return 0;
}


// Source: compress.c
// Lines 139-148
