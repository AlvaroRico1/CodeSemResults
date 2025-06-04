static void dispose_h2o_headers_command(void *_cmds)
{
    h2o_headers_command_t *cmds = _cmds;
    size_t i;
    for (i = 0; cmds[i].cmd != H2O_HEADERS_CMD_NULL; ++i)
        free(cmds[i].args);
}


// Source: headers_util.c
// Lines 117-123
