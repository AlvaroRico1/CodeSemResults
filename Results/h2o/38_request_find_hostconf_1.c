static h2o_hostconf_t *find_hostconf(h2o_hostconf_t **hostconfs, h2o_iovec_t authority, uint16_t default_port,
                                     h2o_iovec_t *wildcard_match)
{
    h2o_iovec_t hostname;
    uint16_t port;
    char *hostname_lc;

    /* safe-guard for alloca */
    if (authority.len >= 65536)
        return NULL;

    /* extract the specified hostname and port */
    if (h2o_url_parse_hostport(authority.base, authority.len, &hostname, &port) == NULL)
        return NULL;
    if (port == 65535)
        port = default_port;

    /* convert supplied hostname to lower-case */
    hostname_lc = alloca(hostname.len);
    h2o_strcopytolower(hostname_lc, hostname.base, hostname.len);

    do {
        h2o_hostconf_t *hostconf = *hostconfs;
        if (hostconf->authority.port == port || (hostconf->authority.port == 65535 && port == default_port)) {
            if (hostconf->authority.host.base[0] == '*') {
                /* matching against "*.foo.bar" */
                size_t cmplen = hostconf->authority.host.len - 1;
                if (cmplen < hostname.len &&
                    memcmp(hostconf->authority.host.base + 1, hostname_lc + hostname.len - cmplen, cmplen) == 0) {
                    *wildcard_match = h2o_iovec_init(hostname.base, hostname.len - cmplen);
                    return hostconf;
                }
            } else {
                /* exact match */
                if (h2o_memis(hostconf->authority.host.base, hostconf->authority.host.len, hostname_lc, hostname.len))
                    return hostconf;
            }
        }
    } while (*++hostconfs != NULL);


// Source: request.c
// Lines 74-112
