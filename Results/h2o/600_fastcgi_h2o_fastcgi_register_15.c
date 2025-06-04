h2o_fastcgi_handler_t *h2o_fastcgi_register(h2o_pathconf_t *pathconf, h2o_url_t *upstream, h2o_fastcgi_config_vars_t *vars)
{
    h2o_fastcgi_handler_t *handler = (void *)h2o_create_handler(pathconf, sizeof(*handler));

    handler->super.on_context_init = on_context_init;
    handler->super.on_context_dispose = on_context_dispose;
    handler->super.dispose = on_handler_dispose;
    handler->super.on_req = on_req;
    handler->config = *vars;
    if (vars->document_root.base != NULL)
        handler->config.document_root = h2o_strdup(NULL, vars->document_root.base, vars->document_root.len);

    h2o_socketpool_target_t *target = h2o_socketpool_create_target(upstream, NULL);
    h2o_socketpool_target_t **targets = &target;
    h2o_socketpool_init_specific(&handler->sockpool, SIZE_MAX /* FIXME */, targets, 1, NULL);
    h2o_socketpool_set_timeout(&handler->sockpool, handler->config.keepalive_timeout);
    return handler;
}


// Source: fastcgi.c
// Lines 831-848
