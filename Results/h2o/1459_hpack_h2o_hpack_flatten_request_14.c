void h2o_hpack_flatten_request(h2o_buffer_t **buf, h2o_hpack_header_table_t *header_table, uint32_t hpack_capacity,
                               uint32_t stream_id, size_t max_frame_size, h2o_iovec_t method, h2o_url_t *url,
                               const h2o_header_t *headers, size_t num_headers, int is_end_stream)
{
    int is_connect = h2o_memis(method.base, method.len, H2O_STRLIT("CONNECT"));

    size_t capacity = calc_headers_capacity(headers, num_headers);
    capacity += H2O_HTTP2_FRAME_HEADER_SIZE;
    capacity += DYNAMIC_TABLE_SIZE_UPDATE_MAX_SIZE;
    capacity += calc_capacity(H2O_TOKEN_METHOD->buf.len, method.len);
    if (!is_connect)
        capacity += calc_capacity(H2O_TOKEN_SCHEME->buf.len, url->scheme->name.len);
    capacity += calc_capacity(H2O_TOKEN_AUTHORITY->buf.len, url->authority.len);
    if (!is_connect)
        capacity += calc_capacity(H2O_TOKEN_PATH->buf.len, url->path.len);

    size_t start_at = (*buf)->size;
    uint8_t *dst = (void *)(h2o_buffer_reserve(buf, capacity).base + H2O_HTTP2_FRAME_HEADER_SIZE);

    /* encode */
    dst = header_table_adjust_size(header_table, hpack_capacity, dst);
    dst = encode_method(header_table, dst, method);
    if (!is_connect)
        dst = encode_scheme(header_table, dst, url->scheme);
    dst = encode_header_token(header_table, dst, H2O_TOKEN_AUTHORITY, &url->authority);
    if (!is_connect)
        dst = encode_path(header_table, dst, url->path);
    for (size_t i = 0; i != num_headers; ++i) {
        const h2o_header_t *header = headers + i;
        if (header->name == &H2O_TOKEN_ACCEPT_ENCODING->buf &&
            h2o_memis(header->value.base, header->value.len, H2O_STRLIT("gzip, deflate"))) {
            *dst++ = 0x90;
        } else {
            dst = encode_header(header_table, dst, header);
        }
    }


// Source: hpack.c
// Lines 952-987
