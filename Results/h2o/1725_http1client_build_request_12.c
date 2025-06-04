static h2o_iovec_t build_request(struct st_h2o_http1client_t *client, h2o_iovec_t method, const h2o_url_t *url,
                                 h2o_iovec_t connection, const h2o_header_t *headers, size_t num_headers)
{
    h2o_iovec_t buf;
    size_t offset = 0;

    buf.len = method.len + url->path.len + url->authority.len + 512;
    buf.base = h2o_mem_alloc_pool(client->super.pool, char, buf.len);

#define RESERVE(sz)                                                                                                                \
    do {                                                                                                                           \
        size_t required = offset + sz + 4 /* for "\r\n\r\n" */;                                                                    \
        if (required > buf.len) {                                                                                                  \
            do {                                                                                                                   \
                buf.len *= 2;                                                                                                      \
            } while (required > buf.len);                                                                                          \
            char *newp = h2o_mem_alloc_pool(client->super.pool, char, buf.len);                                                    \
            memcpy(newp, buf.base, offset);                                                                                        \
            buf.base = newp;                                                                                                       \
        }                                                                                                                          \
    } while (0)
#define APPEND(s, l)                                                                                                               \
    do {                                                                                                                           \
        h2o_memcpy(buf.base + offset, (s), (l));                                                                                   \
        offset += (l);                                                                                                             \
    } while (0)
#define APPEND_STRLIT(lit) APPEND((lit), sizeof(lit) - 1)
#define APPEND_HEADER(h)                                                                                                           \
    do {                                                                                                                           \
        RESERVE((h)->name->len + (h)->value.len + 4);                                                                              \
        APPEND((h)->orig_name ? (h)->orig_name : (h)->name->base, (h)->name->len);                                                 \
        buf.base[offset++] = ':';                                                                                                  \
        buf.base[offset++] = ' ';                                                                                                  \
        APPEND((h)->value.base, (h)->value.len);                                                                                   \
        buf.base[offset++] = '\r';                                                                                                 \
        buf.base[offset++] = '\n';                                                                                                 \
    } while (0)

    APPEND(method.base, method.len);
    buf.base[offset++] = ' ';
    if (client->super.upgrade_to == h2o_httpclient_upgrade_to_connect) {
        if (h2o_memis(method.base, method.len, H2O_STRLIT("CONNECT-UDP"))) {
            APPEND_STRLIT("masque://");
            APPEND(url->authority.base, url->authority.len);
            APPEND_STRLIT("/");
        } else {
            APPEND(url->authority.base, url->authority.len);
        }
    } else {
        APPEND(url->path.base, url->path.len);
    }
    APPEND_STRLIT(" HTTP/1.1\r\nhost: ");
    APPEND(url->authority.base, url->authority.len);
    buf.base[offset++] = '\r';
    buf.base[offset++] = '\n';
    assert(offset <= buf.len);

    if (connection.base != NULL) {
        h2o_header_t h = (h2o_header_t){&H2O_TOKEN_CONNECTION->buf, NULL, connection};
        APPEND_HEADER(&h);
    }

    if (num_headers != 0) {
        for (const h2o_header_t *h = headers, *h_end = h + num_headers; h != h_end; ++h)
            APPEND_HEADER(h);
    }


// Source: http1client.c
// Lines 613-678
