static int on_config_http2_reprioritize_blocking_assets(h2o_configurator_command_t *cmd, h2o_configurator_context_t *ctx,
                                                        yoml_t *node)
{
    struct st_core_configurator_t *self = (void *)cmd->configurator;
    ssize_t on;

    if ((on = h2o_configurator_get_one_of(cmd, node, "OFF,ON")) == -1)
        return -1;
    self->vars->http2.reprioritize_blocking_assets = (int)on;

    return 0;
}


// Source: configurator.c
// Lines 510-521
