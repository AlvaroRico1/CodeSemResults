void h2o_config_dispose(h2o_globalconf_t *config)
{
    size_t i;

    for (i = 0; config->hosts[i] != NULL; ++i) {
        h2o_hostconf_t *hostconf = config->hosts[i];
        destroy_hostconf(hostconf);
    }


// Source: config.c
// Lines 288-295
