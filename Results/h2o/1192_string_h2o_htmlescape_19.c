h2o_iovec_t h2o_htmlescape(h2o_mem_pool_t *pool, const char *src, size_t len)
{
    const char *s, *end = src + len;
    size_t add_size = 0;

#define ENTITY_MAP()                                                                                                               \
    ENTITY('"', "&quot;");                                                                                                         \
    ENTITY('&', "&amp;");                                                                                                          \
    ENTITY('\'', "&#39;");                                                                                                         \
    ENTITY('<', "&lt;");                                                                                                           \
    ENTITY('>', "&gt;");

    for (s = src; s != end; ++s) {
        if ((unsigned)(unsigned char)*s - '"' <= '>' - '"') {
            switch (*s) {
#define ENTITY(code, quoted)                                                                                                       \
    case code:                                                                                                                     \
        add_size += sizeof(quoted) - 2;                                                                                            \
        break
                ENTITY_MAP()
#undef ENTITY
            }
        }
    }

    /* escape and return the result if necessary */
    if (add_size != 0) {
        /* allocate buffer and fill in the chars that are known not to require escaping */
        h2o_iovec_t escaped = {h2o_mem_alloc_pool(pool, char, len + add_size + 1), 0};
        /* fill-in the rest */
        for (s = src; s != end; ++s) {
            switch (*s) {
#define ENTITY(code, quoted)                                                                                                       \
    case code:                                                                                                                     \
        memcpy(escaped.base + escaped.len, quoted, sizeof(quoted) - 1);                                                            \
        escaped.len += sizeof(quoted) - 1;                                                                                         \
        break
                ENTITY_MAP()
#undef ENTITY
            default:
                escaped.base[escaped.len++] = *s;
                break;
            }
        }
        assert(escaped.len == len + add_size);
        escaped.base[escaped.len] = '\0';

        return escaped;
    }

#undef ENTITY_MAP

    /* no need not escape; return the original */
    return h2o_iovec_init(src, len);
}


// Source: string.c
// Lines 477-531
