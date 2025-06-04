static h2o_headers_command_t **get_headers_commands(h2o_configurator_t *_self)
{
    struct headers_configurator_t *self = (void *)_self;
    return self->cmds;
}


// Source: headers.c
// Lines 59-63
