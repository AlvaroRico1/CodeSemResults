static int on_config_document_root(h2o_configurator_command_t *cmd, h2o_configurator_context_t *ctx, yoml_t *node)
{
    struct fastcgi_configurator_t *self = (void *)cmd->configurator;

    if (node->data.scalar[0] == '\0') {
        /* unset */
        self->vars->document_root = h2o_iovec_init(NULL, 0);
    } else if (node->data.scalar[0] == '/') {
        /* set */
        self->vars->document_root = h2o_iovec_init(node->data.scalar, strlen(node->data.scalar));
    } else {
        h2o_configurator_errprintf(cmd, node, "value does not start from `/`");
        return -1;
    }
    return 0;
}


// Source: fastcgi.c
// Lines 54-69
