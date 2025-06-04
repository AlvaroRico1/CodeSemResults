static int on_config_gzip(h2o_configurator_command_t *cmd, h2o_configurator_context_t *ctx, yoml_t *node)
{
    struct compress_configurator_t *self = (void *)cmd->configurator;
    int mode;

    if ((mode = (int)h2o_configurator_get_one_of(cmd, node, "OFF,ON")) == -1)
        return -1;

    *self->vars = all_off;
    if (mode != 0)
        self->vars->gzip.quality = DEFAULT_GZIP_QUALITY;

    return 0;
}


// Source: compress.c
// Lines 35-48
