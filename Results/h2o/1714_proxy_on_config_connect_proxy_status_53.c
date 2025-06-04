static int on_config_connect_proxy_status(h2o_configurator_command_t *cmd, h2o_configurator_context_t *ctx, yoml_t *node)
{
    struct proxy_configurator_t *self = (void *)cmd->configurator;
    ssize_t ret = h2o_configurator_get_one_of(cmd, node, "OFF,ON");
    if (ret == -1)
        return -1;
    self->vars->conf.connect_proxy_status_enabled = (int)ret;
    return 0;
}


// Source: proxy.c
// Lines 114-122
