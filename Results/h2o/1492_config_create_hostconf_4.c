static h2o_hostconf_t *create_hostconf(h2o_globalconf_t *globalconf)
{
    h2o_hostconf_t *hostconf = h2o_mem_alloc(sizeof(*hostconf));
    *hostconf = (h2o_hostconf_t){globalconf};
    hostconf->http2.push_preload = 1; /* enabled by default */
    h2o_config_init_pathconf(&hostconf->fallback_path, globalconf, NULL, globalconf->mimemap);
    hostconf->mimemap = globalconf->mimemap;
    h2o_mem_addref_shared(hostconf->mimemap);
    return hostconf;
}


// Source: config.c
// Lines 36-45
