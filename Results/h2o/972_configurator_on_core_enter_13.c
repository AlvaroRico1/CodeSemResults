static int on_core_enter(h2o_configurator_t *_self, h2o_configurator_context_t *ctx, yoml_t *node)
{
    struct st_core_configurator_t *self = (void *)_self;

    ++self->vars;
    self->vars[0] = self->vars[-1];
    return 0;
}


// Source: configurator.c
// Lines 73-80
