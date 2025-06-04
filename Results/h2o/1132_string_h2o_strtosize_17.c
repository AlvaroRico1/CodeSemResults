size_t h2o_strtosize(const char *s, size_t len)
{
    uint64_t v = 0, m = 1;
    const char *p = s + len;

    if (len == 0)
        goto Error;

    while (1) {
        int ch = *--p;
        if (!('0' <= ch && ch <= '9'))
            goto Error;
        v += (ch - '0') * m;
        if (p == s)
            break;
        m *= 10;
        /* do not even try to overflow */
        if (m == 10000000000000000000ULL)
            goto Error;
    }

    if (v >= SIZE_MAX)
        goto Error;
    return v;

Error:
    return SIZE_MAX;
}


// Source: string.c
// Lines 86-113
