static int on_config_send_delegated_uri(h2o_configurator_command_t *cmd, h2o_configurator_context_t *ctx, yoml_t *node)
{
    struct fastcgi_configurator_t *self = (void *)cmd->configurator;
    ssize_t v;

    if ((v = h2o_configurator_get_one_of(cmd, node, "OFF,ON")) == -1)
        return -1;
    self->vars->send_delegated_uri = (int)v;
    return 0;
}


// Source: fastcgi.c
// Lines 71-80
