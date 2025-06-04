static int on_config_enter(h2o_configurator_t *_self, h2o_configurator_context_t *ctx, yoml_t *node)
{
    struct errordoc_configurator_t *self = (void *)_self;

    if (self->vars == self->_vars_stack) {
        /* entering global level */
        h2o_mem_init_pool(&self->pool);
    }

    /* copy vars */
    memset(&self->vars[1], 0, sizeof(self->vars[1]));
    h2o_vector_reserve(&self->pool, &self->vars[1], self->vars[0].size);
    h2o_memcpy(self->vars[1].entries, self->vars[0].entries, sizeof(self->vars[0].entries[0]) * self->vars[0].size);
    self->vars[1].size = self->vars[0].size;

    ++self->vars;
    return 0;
}


// Source: errordoc.c
// Lines 113-130
