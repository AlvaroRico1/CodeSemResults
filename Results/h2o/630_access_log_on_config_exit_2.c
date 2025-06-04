static int on_config_exit(h2o_configurator_t *_self, h2o_configurator_context_t *ctx, yoml_t *node)
{
    struct st_h2o_access_log_configurator_t *self = (void *)_self;
    size_t i;

    /* register all handles, and decref them */
    for (i = 0; i != self->handles->size; ++i) {
        h2o_access_log_filehandle_t *fh = self->handles->entries[i];
        if (ctx->pathconf != NULL)
            h2o_access_log_register(ctx->pathconf, fh);
        h2o_mem_release_shared(fh);
    }


// Source: access_log.c
// Lines 96-107
