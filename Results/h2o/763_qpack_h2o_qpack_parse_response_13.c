int h2o_qpack_parse_response(h2o_mem_pool_t *pool, h2o_qpack_decoder_t *qpack, int64_t stream_id, int *status,
                             h2o_headers_t *headers, h2o_iovec_t *datagram_flow_id, uint8_t *outbuf, size_t *outbufsize,
                             const uint8_t *_src, size_t len, const char **err_desc)
{
    struct st_h2o_qpack_decode_header_ctx_t ctx;
    const uint8_t *src = _src, *src_end = src + len;
    int ret;

    if ((ret = parse_decode_context(qpack, &ctx, stream_id, &src, src_end)) != 0)
        return ret;
    if ((ret = h2o_hpack_parse_response(pool, decode_header, &ctx, status, headers, datagram_flow_id, src, src_end - src,
                                        err_desc)) != 0)
        return normalize_error_code(ret);

    *outbufsize = send_header_ack(qpack, &ctx, outbuf, stream_id);
    return 0;
}


// Source: qpack.c
// Lines 852-868
