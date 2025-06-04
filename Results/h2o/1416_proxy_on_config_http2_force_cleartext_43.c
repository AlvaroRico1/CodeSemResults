static int on_config_http2_force_cleartext(h2o_configurator_command_t *cmd, h2o_configurator_context_t *ctx, yoml_t *node)
{
    struct proxy_configurator_t *self = (void *)cmd->configurator;
    ssize_t ret = h2o_configurator_get_one_of(cmd, node, "OFF,ON");
    if (ret < 0)
        return -1;
    self->vars->conf.http2.force_cleartext = (unsigned)ret;
    return 0;
}


// Source: proxy.c
// Lines 507-515
