static int on_config_connect_proxy(h2o_configurator_command_t *cmd, h2o_configurator_context_t *ctx, yoml_t *node)
{
    struct proxy_configurator_t *self = (void *)cmd->configurator;

    /* Convert list of ACLs to internal representation; input is a sequence of: [+-]address(?::port|) */
    h2o_connect_acl_entry_t acl_entries[node->data.sequence.size];
    for (size_t i = 0; i < node->data.sequence.size; ++i) {
        if (node->data.sequence.elements[i]->type != YOML_TYPE_SCALAR) {
            h2o_configurator_errprintf(cmd, node->data.sequence.elements[i], "ACL entry must be a scalar");
            return -1;
        }
        const char *err = h2o_connect_parse_acl(acl_entries + i, node->data.sequence.elements[i]->data.scalar);
        if (err != NULL) {
            h2o_configurator_errprintf(cmd, node->data.sequence.elements[i], "%s", err);
            return -1;
        }
    }

    h2o_connect_register(ctx->pathconf, &self->vars->conf, acl_entries, node->data.sequence.size);
    return 0;
}


// Source: proxy.c
// Lines 437-457
