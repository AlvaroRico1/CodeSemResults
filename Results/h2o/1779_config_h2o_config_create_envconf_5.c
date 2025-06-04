h2o_envconf_t *h2o_config_create_envconf(h2o_envconf_t *parent)
{
    h2o_envconf_t *envconf = h2o_mem_alloc_shared(NULL, sizeof(*envconf), on_dispose_envconf);
    *envconf = (h2o_envconf_t){NULL};

    if (parent != NULL) {
        envconf->parent = parent;
        h2o_mem_addref_shared(parent);
    }
    return envconf;
}


// Source: config.c
// Lines 82-92
