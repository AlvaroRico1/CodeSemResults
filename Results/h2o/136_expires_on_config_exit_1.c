static int on_config_exit(h2o_configurator_t *_self, h2o_configurator_context_t *ctx, yoml_t *node)
{
    struct expires_configurator_t *self = (void *)_self;

    if (*self->args != NULL) {
        /* setup */
        if (ctx->pathconf != NULL) {
            h2o_expires_register(ctx->pathconf, *self->args);
        }
        /* destruct */
        assert((*self->args)->mode == H2O_EXPIRES_MODE_MAX_AGE);
        free(*self->args);
        *self->args = NULL;
    }

    --self->args;
    return 0;
}


// Source: expires.c
// Lines 92-109
