static void on_dispose_envconf(void *_envconf)
{
    h2o_envconf_t *envconf = _envconf;
    size_t i;

    if (envconf->parent != NULL)
        h2o_mem_release_shared(envconf->parent);

    for (i = 0; i != envconf->unsets.size; ++i)
        h2o_mem_release_shared(envconf->unsets.entries[i].base);
    free(envconf->unsets.entries);
    for (i = 0; i != envconf->sets.size; ++i)
        h2o_mem_release_shared(envconf->sets.entries[i].base);
    free(envconf->sets.entries);
}


// Source: config.c
// Lines 66-80
