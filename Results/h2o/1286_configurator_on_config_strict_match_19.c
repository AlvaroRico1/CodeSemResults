static int on_config_strict_match(h2o_configurator_command_t *cmd, h2o_configurator_context_t *ctx, yoml_t *node)
{
    h2o_hostconf_t *hostconf = ctx->hostconf;
    ssize_t on;

    if ((on = h2o_configurator_get_one_of(cmd, node, "OFF,ON")) == -1)
        return -1;
    hostconf->strict_match = (uint8_t)on;
    return 0;
}


// Source: configurator.c
// Lines 398-407
