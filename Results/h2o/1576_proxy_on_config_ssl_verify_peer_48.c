static int on_config_ssl_verify_peer(h2o_configurator_command_t *cmd, h2o_configurator_context_t *ctx, yoml_t *node)
{
    struct proxy_configurator_t *self = (void *)cmd->configurator;
    ssize_t ret = h2o_configurator_get_one_of(cmd, node, "OFF,ON");
    if (ret == -1)
        return -1;

    update_ssl_ctx(&self->vars->ssl_ctx, NULL, ret != 0 ? SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT : SSL_VERIFY_NONE,
                   NULL);

    return 0;
}


// Source: proxy.c
// Lines 213-224
