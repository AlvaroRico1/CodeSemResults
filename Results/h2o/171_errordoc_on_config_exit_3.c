static int on_config_exit(h2o_configurator_t *_self, h2o_configurator_context_t *ctx, yoml_t *node)
{
    struct errordoc_configurator_t *self = (void *)_self;

    if (ctx->pathconf != NULL && self->vars->size != 0)
        h2o_errordoc_register(ctx->pathconf, self->vars->entries, self->vars->size);

    --self->vars;
    if (self->vars == self->_vars_stack) {
        /* exitting global level */
        h2o_mem_clear_pool(&self->pool);
    }

    return 0;
}


// Source: errordoc.c
// Lines 132-146
