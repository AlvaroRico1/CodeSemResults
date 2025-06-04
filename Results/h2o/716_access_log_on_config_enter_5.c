static int on_config_enter(h2o_configurator_t *_self, h2o_configurator_context_t *ctx, yoml_t *node)
{
    struct st_h2o_access_log_configurator_t *self = (void *)_self;
    size_t i;

    /* push the stack pointer */
    ++self->handles;

    /* link the handles */
    memset(self->handles, 0, sizeof(*self->handles));
    h2o_vector_reserve(NULL, self->handles, self->handles[-1].size + 1);
    for (i = 0; i != self->handles[-1].size; ++i) {
        h2o_access_log_filehandle_t *fh = self->handles[-1].entries[i];
        self->handles[0].entries[self->handles[0].size++] = fh;
        h2o_mem_addref_shared(fh);
    }

    return 0;
}


// Source: access_log.c
// Lines 76-94
