static h2o_hostconf_t *find_default_hostconf(h2o_hostconf_t **hostconfs)
{
    h2o_hostconf_t *fallback_host = hostconfs[0]->global->fallback_host;

    do {
        h2o_hostconf_t *hostconf = *hostconfs;
        if (!hostconf->strict_match)
            return hostconf;
    } while (*++hostconfs != NULL);

    return fallback_host;
}


// Source: request.c
// Lines 117-128
