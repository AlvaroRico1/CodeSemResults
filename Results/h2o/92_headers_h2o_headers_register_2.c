void h2o_headers_register(h2o_pathconf_t *pathconf, h2o_headers_command_t *cmds)
{
    struct st_headers_filter_t *self = (void *)h2o_create_filter(pathconf, sizeof(*self));

    self->super.on_setup_ostream = on_setup_ostream;
    self->super.on_informational = on_informational;
    self->cmds = cmds;

    if (requires_early_hints_handler(self)) {
        struct st_headers_early_hints_handler_t *handler = (void *)h2o_create_handler(pathconf, sizeof(*handler));
        handler->cmds = cmds;
        handler->super.on_req = on_req;

        /* move this handler to first */
        memmove(pathconf->handlers.entries + 1, pathconf->handlers.entries,
                sizeof(h2o_handler_t *) * (pathconf->handlers.size - 1));
        pathconf->handlers.entries[0] = &handler->super;
    }


// Source: headers.c
// Lines 111-128
