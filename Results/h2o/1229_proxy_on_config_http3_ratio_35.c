static int on_config_http3_ratio(h2o_configurator_command_t *cmd, h2o_configurator_context_t *ctx, yoml_t *node)
{
    struct proxy_configurator_t *self = (void *)cmd->configurator;
    int ret = h2o_configurator_scanf(cmd, node, "%" SCNd8, &self->vars->conf.protocol_ratio.http3);
    if (ret < 0)
        return ret;
    if (self->vars->conf.protocol_ratio.http3 < 0 || 100 < self->vars->conf.protocol_ratio.http3) {
        h2o_configurator_errprintf(cmd, node, "proxy.http3.ratio must be between 0 and 100");
        return -1;
    }
    return 0;
}


// Source: proxy.c
// Lines 530-541
