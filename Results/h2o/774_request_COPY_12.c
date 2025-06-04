#define COPY(buf)                                                                                                                  \
    do {                                                                                                                           \
        req->buf.base = h2o_mem_alloc_pool(&req->pool, char, src->buf.len);                                                        \
        memcpy(req->buf.base, src->buf.base, src->buf.len);                                                                        \
        req->buf.len = src->buf.len;                                                                                               \
    } while (0)
        COPY(input.authority);
        COPY(input.method);
        COPY(input.path);
        req->input.scheme = src->input.scheme;
        req->version = src->version;
        req->entity = src->entity;
        req->http1_is_persistent = src->http1_is_persistent;
        req->timestamps = src->timestamps;
        if (src->upgrade.base != NULL) {
            COPY(upgrade);
        } else {
            req->upgrade.base = NULL;
            req->upgrade.len = 0;
        }
#undef COPY
        h2o_vector_reserve(&req->pool, &req->headers, src->headers.size);
        req->headers.size = src->headers.size;
        for (i = 0; i != src->headers.size; ++i) {
            h2o_header_t *dst_header = req->headers.entries + i, *src_header = src->headers.entries + i;
            if (h2o_iovec_is_token(src_header->name)) {
                dst_header->name = src_header->name;
            } else {
                dst_header->name = h2o_mem_alloc_pool(&req->pool, *dst_header->name, 1);
                *dst_header->name = h2o_strdup(&req->pool, src_header->name->base, src_header->name->len);
            }
            dst_header->value = h2o_strdup(&req->pool, src_header->value.base, src_header->value.len);
            dst_header->flags = src_header->flags;
            if (!src_header->orig_name)
                dst_header->orig_name = NULL;
            else
                dst_header->orig_name = h2o_strdup(&req->pool, src_header->orig_name, src_header->name->len).base;
        }


// Source: request.c
// Lines 277-314
