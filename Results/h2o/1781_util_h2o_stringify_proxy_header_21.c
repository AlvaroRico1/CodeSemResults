size_t h2o_stringify_proxy_header(h2o_conn_t *conn, char *buf)
{
    struct sockaddr_storage ss;
    socklen_t sslen;
    size_t strlen;
    uint16_t peerport;
    char *dst = buf;

    if ((sslen = conn->callbacks->get_peername(conn, (void *)&ss)) == 0)
        goto Unknown;
    switch (ss.ss_family) {
    case AF_INET:
        memcpy(dst, "PROXY TCP4 ", 11);
        dst += 11;
        break;
    case AF_INET6:
        memcpy(dst, "PROXY TCP6 ", 11);
        dst += 11;
        break;
    default:
        goto Unknown;
    }
    if ((strlen = h2o_socket_getnumerichost((void *)&ss, sslen, dst)) == SIZE_MAX)
        goto Unknown;
    dst += strlen;
    *dst++ = ' ';

    peerport = h2o_socket_getport((void *)&ss);

    if ((sslen = conn->callbacks->get_sockname(conn, (void *)&ss)) == 0)
        goto Unknown;
    if ((strlen = h2o_socket_getnumerichost((void *)&ss, sslen, dst)) == SIZE_MAX)
        goto Unknown;
    dst += strlen;
    *dst++ = ' ';

    dst += sprintf(dst, "%" PRIu16 " %" PRIu16 "\r\n", peerport, (uint16_t)h2o_socket_getport((void *)&ss));

    return dst - buf;

Unknown:
    memcpy(buf, "PROXY UNKNOWN\r\n", 15);
    return 15;
}


// Source: util.c
// Lines 551-594
