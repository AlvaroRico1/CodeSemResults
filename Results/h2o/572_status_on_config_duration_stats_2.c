static int on_config_duration_stats(h2o_configurator_command_t *cmd, h2o_configurator_context_t *ctx, yoml_t *node)
{
    struct st_status_configurator *c = (void *)cmd->configurator;
    ssize_t ret;
    switch (ret = h2o_configurator_get_one_of(cmd, node, "OFF,ON")) {
    case 0: /* OFF */
    case 1: /* ON */
        c->duration_stats = (int)ret;
        return 0;
    default: /* error */
        return -1;
    }
}


// Source: status.c
// Lines 44-56
