static int on_config_connect(h2o_configurator_command_t *cmd, h2o_configurator_context_t *ctx, yoml_t *node)
{
    struct fastcgi_configurator_t *self = (void *)cmd->configurator;
    const char *hostname = "127.0.0.1", *servname = NULL, *type = "tcp";

    /* fetch servname (and hostname) */
    switch (node->type) {
    case YOML_TYPE_SCALAR:
        servname = node->data.scalar;
        break;
    case YOML_TYPE_MAPPING: {
        yoml_t **port_node, **host_node, **type_node;
        if (h2o_configurator_parse_mapping(cmd, node, "port:s", "host:s,type:s", &port_node, &host_node, &type_node) != 0)
            return -1;
        servname = (*port_node)->data.scalar;
        if (host_node != NULL)
            hostname = (*host_node)->data.scalar;
        if (type_node != NULL)
            type = (*type_node)->data.scalar;
    } break;
    default:
        h2o_configurator_errprintf(cmd, node,
                                   "value must be a string or a mapping (with keys: `port` and optionally `host` and `type`)");
        return -1;
    }

    h2o_url_t upstream;

    if (strcmp(type, "unix") == 0) {
        /* unix socket */
        struct sockaddr_un sa;
        if (strlen(servname) >= sizeof(sa.sun_path)) {
            h2o_configurator_errprintf(cmd, node, "path:%s is too long as a unix socket name", servname);
            return -1;
        }
        h2o_url_init_with_sun_path(&upstream, NULL, &H2O_URL_SCHEME_FASTCGI, h2o_iovec_init(servname, strlen(servname)),
                                   h2o_iovec_init(H2O_STRLIT("/")));
    } else if (strcmp(type, "tcp") == 0) {
        /* tcp socket */
        uint16_t port;
        if (sscanf(servname, "%" SCNu16, &port) != 1) {
            h2o_configurator_errprintf(cmd, node, "invalid port number:%s", servname);
            return -1;
        }
        h2o_url_init_with_hostport(&upstream, NULL, &H2O_URL_SCHEME_FASTCGI, h2o_iovec_init(hostname, strlen(hostname)), port,
                                   h2o_iovec_init(H2O_STRLIT("/")));
    } else {
        h2o_configurator_errprintf(cmd, node, "unknown listen type: %s", type);
        return -1;
    }

    h2o_fastcgi_register(ctx->pathconf, &upstream, self->vars);
    free(upstream.authority.base);

    return 0;
}


// Source: fastcgi.c
// Lines 82-137
