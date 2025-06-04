static void on_informational(h2o_filter_t *_self, h2o_req_t *req)
{
    struct st_headers_filter_t *self = (void *)_self;
    h2o_headers_command_t *cmd;

    if (req->res.status != 103)
        return;

    for (cmd = self->cmds; cmd->cmd != H2O_HEADERS_CMD_NULL; ++cmd) {
        if (cmd->when != H2O_HEADERS_CMD_WHEN_FINAL)
            h2o_rewrite_headers(&req->pool, &req->res.headers, cmd);
    }
}


// Source: headers.c
// Lines 53-65
