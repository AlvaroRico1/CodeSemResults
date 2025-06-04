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
// Lines 640-678
