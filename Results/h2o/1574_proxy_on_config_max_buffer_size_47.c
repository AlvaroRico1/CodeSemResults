static int on_config_max_buffer_size(h2o_configurator_command_t *cmd, h2o_configurator_context_t *ctx, yoml_t *node)
{
    struct proxy_configurator_t *self = (void *)cmd->configurator;
    return h2o_configurator_scanf(cmd, node, "%zu", &self->vars->conf.max_buffer_size);
}


// Source: proxy.c
// Lines 495-499
