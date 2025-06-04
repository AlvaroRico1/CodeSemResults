static int on_config_exit(h2o_configurator_t *_self, h2o_configurator_context_t *ctx, yoml_t *node)
{
    struct headers_configurator_t *self = (void *)_self;

    if (ctx->pathconf != NULL && *self->cmds != NULL) {
        if (*self->cmds != NULL)
            h2o_mem_addref_shared(*self->cmds);
        h2o_headers_register(ctx->pathconf, *self->cmds);
    }

    if (*self->cmds != NULL)
        h2o_mem_release_shared(*self->cmds);
    --self->cmds;
    return 0;
}


// Source: headers.c
// Lines 43-57
