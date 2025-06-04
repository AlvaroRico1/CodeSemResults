static int on_config_ssl_cafile(h2o_configurator_command_t *cmd, h2o_configurator_context_t *ctx, yoml_t *node)
{
    struct proxy_configurator_t *self = (void *)cmd->configurator;
    X509_STORE *store = X509_STORE_new();
    int ret = -1;

    if (X509_STORE_load_locations(store, node->data.scalar, NULL) == 1) {
        update_ssl_ctx(&self->vars->ssl_ctx, store, -1, NULL);
        ret = 0;
    } else {
        h2o_configurator_errprintf(cmd, node, "failed to load certificates file:%s", node->data.scalar);
        ERR_print_errors_fp(stderr);
    }

    X509_STORE_free(store);
    return ret;
}


// Source: proxy.c
// Lines 226-242
