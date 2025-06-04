static int on_config_enter(h2o_configurator_t *_self, h2o_configurator_context_t *ctx, yoml_t *node)
{
    struct expires_configurator_t *self = (void *)_self;

    if (self->args[0] != NULL) {
        /* duplicate */
        assert(self->args[0]->mode == H2O_EXPIRES_MODE_MAX_AGE);
        self->args[1] = h2o_mem_alloc(sizeof(**self->args));
        *self->args[1] = *self->args[0];
    } else {
        self->args[1] = NULL;
    }
    ++self->args;
    return 0;
}


// Source: expires.c
// Lines 76-90
