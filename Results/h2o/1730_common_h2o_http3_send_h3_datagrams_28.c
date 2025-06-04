void h2o_http3_send_h3_datagrams(h2o_http3_conn_t *conn, uint64_t flow_id, h2o_iovec_t *datagrams, size_t num_datagrams)
{
    for (size_t i = 0; i < num_datagrams; ++i) {
        h2o_iovec_t *src = datagrams + i;
        uint8_t buf[quicly_encodev_capacity(flow_id) + src->len], *p = buf;
        p = quicly_encodev(p, flow_id);
        memcpy(p, src->base, src->len);
        p += src->len;
        ptls_iovec_t payload = ptls_iovec_init(buf, p - buf);
        quicly_send_datagram_frames(conn->super.quic, &payload, 1);
    }


// Source: common.c
// Lines 1333-1343
