static void ingress_unistream_on_receive(quicly_stream_t *qs, size_t off, const void *input, size_t len)
{
    h2o_http3_conn_t *conn = *quicly_get_data(qs->conn);
    struct st_h2o_http3_ingress_unistream_t *stream = qs->data;

    /* save received data */
    h2o_http3_update_recvbuf(&stream->recvbuf, off, input, len);

    /* determine bytes that can be handled */
    const uint8_t *src = (const uint8_t *)stream->recvbuf->bytes,
                  *src_end = src + quicly_recvstate_bytes_available(&stream->quic->recvstate);
    if (src == src_end && !quicly_recvstate_transfer_complete(&stream->quic->recvstate))
        return;

    /* handle the bytes */
    stream->handle_input(conn, stream, &src, src_end, quicly_recvstate_transfer_complete(&stream->quic->recvstate));
    if (quicly_get_state(conn->super.quic) >= QUICLY_STATE_CLOSING)
        return;

    /* remove bytes that have been consumed */
    size_t bytes_consumed = src - (const uint8_t *)stream->recvbuf->bytes;
    if (bytes_consumed != 0) {
        h2o_buffer_consume(&stream->recvbuf, bytes_consumed);
        quicly_stream_sync_recvbuf(stream->quic, bytes_consumed);
    }
}


// Source: common.c
// Lines 210-235
