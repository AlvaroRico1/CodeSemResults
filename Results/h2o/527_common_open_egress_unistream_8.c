static int open_egress_unistream(h2o_http3_conn_t *conn, struct st_h2o_http3_egress_unistream_t **stream, h2o_iovec_t initial_bytes)
{
    quicly_stream_t *qs;
    int ret;

    if ((ret = quicly_open_stream(conn->super.quic, &qs, 1)) != 0)
        return ret;
    *stream = qs->data;
    assert((*stream)->quic == qs);

    h2o_buffer_append(&(*stream)->sendbuf, initial_bytes.base, initial_bytes.len);
    return quicly_stream_sync_sendbuf((*stream)->quic, 1);
}


// Source: common.c
// Lines 414-426
