static h2o_configurator_context_t *create_context(h2o_configurator_context_t *parent, int is_custom_handler)
{
    h2o_configurator_context_t *ctx = h2o_mem_alloc(sizeof(*ctx));
    if (parent == NULL) {
        *ctx = (h2o_configurator_context_t){NULL};
        return ctx;
    }
    *ctx = *parent;
    if (ctx->env != NULL)
        h2o_mem_addref_shared(ctx->env);
    ctx->parent = parent;
    return ctx;
}


// Source: configurator.c
// Lines 48-60
