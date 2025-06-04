static void on_receive_datagram_frame(quicly_receive_datagram_frame_t *self, quicly_conn_t *qc, ptls_iovec_t datagram)
{
    struct st_h2o_httpclient__h3_conn_t *conn =
        H2O_STRUCT_FROM_MEMBER(struct st_h2o_httpclient__h3_conn_t, super, *quicly_get_data(qc));
    uint64_t flow_id;
    h2o_iovec_t payload;
    quicly_stream_t *qs;

    /* decode, validate, get stream */
    if ((flow_id = h2o_http3_decode_h3_datagram(&payload, datagram.base, datagram.len)) == UINT64_MAX ||
        !(quicly_stream_is_client_initiated(flow_id) && !quicly_stream_is_unidirectional(flow_id))) {
        h2o_quic_close_connection(&conn->super.super, H2O_HTTP3_ERROR_GENERAL_PROTOCOL, "invalid DATAGRAM frame");
        return;
    }
    if ((qs = quicly_get_stream(conn->super.super.quic, flow_id)) == NULL)
        return;

    struct st_h2o_http3client_req_t *req = qs->data;
    if (req->on_read_datagrams != NULL)
        req->on_read_datagrams(&req->super, &payload, 1);
}


// Source: http3client.c
// Lines 907-927
