static int on_config_mime_settypes(h2o_configurator_command_t *cmd, h2o_configurator_context_t *ctx, yoml_t *node)
{
    h2o_mimemap_t *newmap = h2o_mimemap_create();
    h2o_mimemap_clear_types(newmap);
    h2o_mimemap_set_default_type(newmap, h2o_mimemap_get_default_type(*ctx->mimemap)->data.mimetype.base, NULL);
    if (set_mimetypes(cmd, newmap, node) != 0) {
        h2o_mem_release_shared(newmap);
        return -1;
    }

    h2o_mem_release_shared(*ctx->mimemap);
    *ctx->mimemap = newmap;
    return 0;
}


// Source: configurator.c
// Lines 756-769
