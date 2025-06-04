static int on_config_http2_max_concurrent_streams(h2o_configurator_command_t *cmd, h2o_configurator_context_t *ctx, yoml_t *node)
{
    struct proxy_configurator_t *self = (void *)cmd->configurator;
    return h2o_configurator_scanf(cmd, node, "%u", &self->vars->conf.http2.max_concurrent_streams);
}


// Source: proxy.c
// Lines 501-505
