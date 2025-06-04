static int on_config_happy_eyeballs_name_resolution_delay(h2o_configurator_command_t *cmd, h2o_configurator_context_t *ctx,
                                                          yoml_t *node)
{
    struct proxy_configurator_t *self = (void *)cmd->configurator;
    return h2o_configurator_scanf(cmd, node, "%" SCNu64, &self->vars->conf.happy_eyeballs.name_resolution_delay);
}


// Source: proxy.c
// Lines 80-85
