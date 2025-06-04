static h2o_headers_command_t **get_headers_commands(h2o_configurator_t *_self)
{
    struct proxy_configurator_t *self = (void *)_self;
    return &self->vars->conf.headers_cmds;
}


// Source: proxy.c
// Lines 608-612
