static int on_config_timeout_connect(h2o_configurator_command_t *cmd, h2o_configurator_context_t *ctx, yoml_t *node)
{
    struct proxy_configurator_t *self = (void *)cmd->configurator;
    self->connect_timeout_set = 1;
    return h2o_configurator_scanf(cmd, node, "%" SCNu64, &self->vars->conf.connect_timeout);
}


// Source: proxy.c
// Lines 60-65
