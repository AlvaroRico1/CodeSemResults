static int on_config_error_log_emit_request_errors(h2o_configurator_command_t *cmd, h2o_configurator_context_t *ctx, yoml_t *node)
{
    struct st_core_configurator_t *self = (void *)cmd->configurator;
    ssize_t on;

    if ((on = h2o_configurator_get_one_of(cmd, node, "OFF,ON")) == -1)
        return -1;

    self->vars->error_log.emit_request_errors = (int)on;
    return 0;
}


// Source: configurator.c
// Lines 968-978
