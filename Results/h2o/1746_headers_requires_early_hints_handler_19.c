static int requires_early_hints_handler(struct st_headers_filter_t *self)
{
    h2o_headers_command_t *cmd;
    for (cmd = self->cmds; cmd->cmd != H2O_HEADERS_CMD_NULL; ++cmd) {
        if (cmd->cmd != H2O_HEADERS_CMD_UNSET && cmd->when != H2O_HEADERS_CMD_WHEN_FINAL)
            return 1;
    }
    return 0;
}


// Source: headers.c
// Lines 101-109
