static int on_config_exit(h2o_configurator_t *_self, h2o_configurator_context_t *ctx, yoml_t *node)
{
    struct st_h2o_file_configurator_t *self = (void *)_self;
    free(self->vars->index_files);
    --self->vars;
    return 0;
}


// Source: file.c
// Lines 154-160
