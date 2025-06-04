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


// Source: main.c
// Lines 1607-1633
