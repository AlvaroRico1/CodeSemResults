int h2o_configurator_apply(h2o_globalconf_t *config, yoml_t *node, int dry_run)
{
    h2o_configurator_context_t *ctx = create_context(NULL, 0);
    ctx->globalconf = config;
    ctx->mimemap = &ctx->globalconf->mimemap;
    ctx->dry_run = dry_run;
    int cmd_ret = h2o_configurator_apply_commands(ctx, node, H2O_CONFIGURATOR_FLAG_GLOBAL, NULL);
    destroy_context(ctx);

    if (cmd_ret != 0)
        return cmd_ret;
    if (config->hosts[0] == NULL) {
        h2o_configurator_errprintf(NULL, node, "mandatory configuration directive `hosts` is missing");
        return -1;
    }
    return 0;
}


// Source: configurator.c
// Lines 1211-1227
