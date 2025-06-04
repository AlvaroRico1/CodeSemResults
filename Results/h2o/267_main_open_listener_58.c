static int open_listener(int domain, int type, int protocol, struct sockaddr *addr, socklen_t addrlen)
{
    int fd;

    if ((fd = socket(domain, type, protocol)) == -1)
        goto Error;
    set_cloexec(fd);

    /* set SO_*, IP_* options */
#ifdef IPV6_V6ONLY
    if (domain == AF_INET6) {
        int flag = 1;
        if (setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &flag, sizeof(flag)) != 0) {
            perror("setsockopt(IPV6_V6ONLY) failed");
            goto Error;
        }
    }
#endif
    switch (type) {
    case SOCK_STREAM: {
        if (conf.tcp_reuseport)
            socket_reuseport(fd);
        /* TCP: set SO_REUSEADDR flag to avoid TIME_WAIT after shutdown */
        int on = 1;
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) != 0)
            goto Error;
    } break;
    case SOCK_DGRAM:
        /* UDP: set SO_REUSEPORT and DF bit */
        socket_reuseport(fd);
        h2o_socket_set_df_bit(fd, domain);
        break;
    default:
        h2o_fatal("unexpected socket type %d", type);
        break;
    }

    /* bind */
    if (bind(fd, addr, addrlen) != 0)
        goto Error;

    /* TCP-specific actions */
    if (protocol == IPPROTO_TCP) {
#ifdef TCP_DEFER_ACCEPT
        { /* set TCP_DEFER_ACCEPT */
            int flag = 1;
            if (setsockopt(fd, IPPROTO_TCP, TCP_DEFER_ACCEPT, &flag, sizeof(flag)) != 0)
                goto Error;
        }
#endif
        /* listen */
        if (listen(fd, H2O_SOMAXCONN) != 0)
            goto Error;
#ifdef SO_ACCEPTFILTER
        { /* set SO_ACCEPTFILTER */
            struct accept_filter_arg arg = {0};
            strcpy(arg.af_name, "httpready");
            if (setsockopt(fd, SOL_SOCKET, SO_ACCEPTFILTER, &arg, sizeof(arg)) != 0)
                fprintf(stderr, "[warning] failed to set SO_ACCEPTFILTER:%s\n", strerror(errno));
        }
#endif
        /* set TCP_FASTOPEN; when tfo_queues is zero TFO is always disabled */
        if (conf.tfo_queues > 0) {
#ifdef TCP_FASTOPEN
            int tfo_queues;
#ifdef __APPLE__
            /* In OS X, the option value for TCP_FASTOPEN must be 1 if is's enabled */
            tfo_queues = 1;
#else
            tfo_queues = conf.tfo_queues;
#endif
            if (setsockopt(fd, IPPROTO_TCP, TCP_FASTOPEN, (const void *)&tfo_queues, sizeof(tfo_queues)) != 0)
                fprintf(stderr, "[warning] failed to set TCP_FASTOPEN:%s\n", strerror(errno));
#else
            assert(!"conf.tfo_queues not zero on platform without TCP_FASTOPEN");
#endif
        }


// Source: main.c
// Lines 1607-1683
