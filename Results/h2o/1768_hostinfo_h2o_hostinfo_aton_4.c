int h2o_hostinfo_aton(h2o_iovec_t host, struct in_addr *addr)
{
    union {
        int32_t n;
        unsigned char c[4];
    } value;
    const char *p = host.base, *end = p + host.len;
    size_t ndots = 0;

    while (1) {
        if ((p = fetch_aton_digit(p, end, value.c + ndots)) == NULL)
            return -1;
        if (ndots == 3)
            break;
        if (p == end || !(*p == '.'))
            return -1;
        ++p;
        ++ndots;
    }
    if (p != end)
        return -1;

    addr->s_addr = value.n;
    return 0;
}


// Source: hostinfo.c
// Lines 262-286
