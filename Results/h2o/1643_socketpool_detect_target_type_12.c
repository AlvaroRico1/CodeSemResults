h2o_socketpool_target_type_t detect_target_type(h2o_url_t *url, struct sockaddr_storage *sa, socklen_t *salen)
{
    memset(sa, 0, sizeof(*sa));
    const char *to_sun_err = h2o_url_host_to_sun(url->host, (struct sockaddr_un *)sa);
    if (to_sun_err == h2o_url_host_to_sun_err_is_not_unix_socket) {
        sa->ss_family = AF_INET;
        struct sockaddr_in *sin = (struct sockaddr_in *)sa;
        *salen = sizeof(*sin);

        if (h2o_hostinfo_aton(url->host, &sin->sin_addr) == 0) {
            sin->sin_port = htons(h2o_url_get_port(url));
            return H2O_SOCKETPOOL_TYPE_SOCKADDR;
        } else {
            return H2O_SOCKETPOOL_TYPE_NAMED;
        }
    } else {
        assert(to_sun_err == NULL);
        *salen = sizeof(struct sockaddr_un);
        return H2O_SOCKETPOOL_TYPE_SOCKADDR;
    }


// Source: socketpool.c
// Lines 150-169
