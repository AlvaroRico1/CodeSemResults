static void set_quic_sockopts(int fd, int family, unsigned sndbuf, unsigned rcvbuf)
{
    /* set the option for obtaining destination address */
    switch (family) {
    case AF_INET: {
#if defined(IP_PKTINFO) /* this is the de-facto API (that works on both linux, macOS) */
        int on = 1;
        if (setsockopt(fd, IPPROTO_IP, IP_PKTINFO, &on, sizeof(on)) != 0)
            h2o_fatal("failed to set IP_PKTINFO option:%s", strerror(errno));
#elif defined(IP_RECVDSTADDR) /* *BSD */
        int on = 1;
        if (setsockopt(fd, IPPROTO_IP, IP_RECVDSTADDR, &on, sizeof(on)) != 0)
            h2o_fatal("failed to set IP_RECVDSTADDR option:%s", strerror(errno));
#endif
    } break;


// Source: main.c
// Lines 1706-1720
