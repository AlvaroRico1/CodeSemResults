static int on_config_enter(h2o_configurator_t *_self, h2o_configurator_context_t *ctx, yoml_t *node)
{
    struct headers_configurator_t *self = (void *)_self;

    self->cmds[1] = self->cmds[0];
    if (self->cmds[1] != NULL)
        h2o_mem_addref_shared(self->cmds[1]);

    ++self->cmds;
    return 0;
}


// Source: headers.c
// Lines 31-41
