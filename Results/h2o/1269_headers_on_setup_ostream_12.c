static void on_setup_ostream(h2o_filter_t *_self, h2o_req_t *req, h2o_ostream_t **slot)
{
    struct st_headers_filter_t *self = (void *)_self;
    h2o_headers_command_t *cmd;

    for (cmd = self->cmds; cmd->cmd != H2O_HEADERS_CMD_NULL; ++cmd) {
        if (cmd->when != H2O_HEADERS_CMD_WHEN_EARLY)
            h2o_rewrite_headers(&req->pool, &req->res.headers, cmd);
    }

    h2o_setup_next_ostream(req, slot);
}


// Source: headers.c
// Lines 40-51
