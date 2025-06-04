static int config_path(h2o_configurator_context_t *parent_ctx, h2o_pathconf_t *pathconf, yoml_t *node)
{
    h2o_configurator_context_t *path_ctx = create_context(parent_ctx, 0);
    path_ctx->pathconf = pathconf;
    path_ctx->mimemap = &pathconf->mimemap;

    int ret = h2o_configurator_apply_commands(path_ctx, node, H2O_CONFIGURATOR_FLAG_PATH, NULL);

    destroy_context(path_ctx);
    return ret;
}


// Source: configurator.c
// Lines 304-314
