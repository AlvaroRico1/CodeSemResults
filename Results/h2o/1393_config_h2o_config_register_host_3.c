h2o_hostconf_t *h2o_config_register_host(h2o_globalconf_t *config, h2o_iovec_t host, uint16_t port)
{
    h2o_hostconf_t *hostconf = NULL;
    h2o_iovec_t host_lc;

    assert(host.len != 0);

    /* convert hostname to lowercase */
    host_lc = h2o_strdup(NULL, host.base, host.len);
    h2o_strtolower(host_lc.base, host_lc.len);

    { /* return NULL if given authority is already registered */
        h2o_hostconf_t **p;
        for (p = config->hosts; *p != NULL; ++p)
            if (h2o_memis((*p)->authority.host.base, (*p)->authority.host.len, host_lc.base, host_lc.len) &&
                (*p)->authority.port == port)
                goto Exit;
    }


// Source: config.c
// Lines 243-260
