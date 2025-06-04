static int on_config_throttle_resp(h2o_configurator_command_t *cmd, h2o_configurator_context_t *ctx, yoml_t *node)
{
    struct throttle_resp_configurator_t *self = (void *)cmd->configurator;

    if ((self->vars->on = (int)h2o_configurator_get_one_of(cmd, node, "OFF,ON")) == -1)
        return -1;
    return 0;
}


// Source: throttle_resp.c
// Lines 34-41
