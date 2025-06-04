static int on_config_http2_allow_cross_origin_push(h2o_configurator_command_t *cmd, h2o_configurator_context_t *ctx, yoml_t *node)
{
    struct st_core_configurator_t *self = (void *)cmd->configurator;
    ssize_t on;

    if ((on = h2o_configurator_get_one_of(cmd, node, "OFF,ON")) == -1)
        return -1;
    self->vars->http2.allow_cross_origin_push = (int)on;

    return 0;
}


// Source: configurator.c
// Lines 535-545
