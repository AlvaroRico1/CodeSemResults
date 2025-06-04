static int on_config_file(h2o_configurator_command_t *cmd, h2o_configurator_context_t *ctx, yoml_t *node)
{
    struct st_h2o_file_configurator_t *self = (void *)cmd->configurator;
    h2o_mimemap_type_t *mime_type =
        h2o_mimemap_get_type_by_extension(*ctx->mimemap, h2o_get_filext(node->data.scalar, strlen(node->data.scalar)));
    h2o_file_register_file(ctx->pathconf, node->data.scalar, mime_type, self->vars->flags);
    return 0;
}


// Source: file.c
// Lines 44-51
