static size_t flatten_res_headers(char *buf, h2o_req_t *req)
{
    char *dst = buf;
    size_t i;
    for (i = 0; i != req->res.headers.size; ++i) {
        const h2o_header_t *header = req->res.headers.entries + i;
        memcpy(dst, header->orig_name ? header->orig_name : header->name->base, header->name->len);
        dst += header->name->len;
        *dst++ = ':';
        *dst++ = ' ';
        memcpy(dst, header->value.base, header->value.len);
        dst += header->value.len;
        *dst++ = '\r';
        *dst++ = '\n';
    }

    return dst - buf;
}


// Source: http1.c
// Lines 888-905
