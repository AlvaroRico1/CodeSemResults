static int on_config_server_timing(h2o_configurator_command_t *cmd, h2o_configurator_context_t *ctx, yoml_t *node)
{
    struct server_timing_configurator_t *self = (void *)cmd->configurator;

    ssize_t ret = h2o_configurator_get_one_of(cmd, node, "OFF,ON,ENFORCE");
    if (ret == -1)
        return -1;
    self->vars->mode = (int)ret;

    return 0;
}


// Source: server_timing.c
// Lines 37-47
