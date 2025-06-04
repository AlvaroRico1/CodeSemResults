void h2o_headers_append_command(h2o_headers_command_t **cmds, int cmd, h2o_headers_command_arg_t *args, size_t num_args,
                                h2o_headers_command_when_t when)
{
    h2o_headers_command_t *new_cmds;
    size_t i, cnt;

    if (*cmds != NULL) {
        for (cnt = 0; (*cmds)[cnt].cmd != H2O_HEADERS_CMD_NULL; ++cnt)
            ;
    } else {
        cnt = 0;
    }

    new_cmds = h2o_mem_alloc_shared(NULL, (cnt + 2) * sizeof(*new_cmds), dispose_h2o_headers_command);
    if (*cmds != NULL)
        memcpy(new_cmds, *cmds, cnt * sizeof(*new_cmds));
    new_cmds[cnt] = (h2o_headers_command_t){};
    new_cmds[cnt].cmd = cmd;
    new_cmds[cnt].when = when;
    new_cmds[cnt].args = h2o_mem_alloc(sizeof(*new_cmds->args) * num_args);
    for (i = 0; i < num_args; i++)
        new_cmds[cnt].args[i] = args[i];
    new_cmds[cnt].num_args = num_args;
    new_cmds[cnt + 1] = (h2o_headers_command_t){H2O_HEADERS_CMD_NULL};

    if (*cmds != NULL) {
        (*cmds)[0] = (h2o_headers_command_t){H2O_HEADERS_CMD_NULL};
        h2o_mem_release_shared(*cmds);
    }
    *cmds = new_cmds;
}


// Source: headers_util.c
// Lines 125-155
