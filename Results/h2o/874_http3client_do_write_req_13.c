int do_write_req(h2o_httpclient_t *_client, h2o_iovec_t chunk, int is_end_stream)
{
    struct st_h2o_http3client_req_t *req = (void *)_client;

    assert(req->proceed_req.bytes_inflight == SIZE_MAX);

    /* Notify error to the application, if the stream has already been closed (due to e.g., a stream error) or if the send-side has
     * been closed (due to STOP_SENDING). Also, destroy the request if the receive side has already been closed. */
    if (req->quic == NULL || !quicly_sendstate_is_open(&req->quic->sendstate)) {
        if (req->quic != NULL && quicly_recvstate_transfer_complete(&req->quic->recvstate))
            close_stream(req, H2O_HTTP3_ERROR_REQUEST_CANCELLED);
        if (req->quic == NULL)
            destroy_request(req);
        return 1;
    }

    emit_data(req, chunk);

    /* shutdown if we've written all request body */
    if (is_end_stream) {
        assert(quicly_sendstate_is_open(&req->quic->sendstate));
        quicly_sendstate_shutdown(&req->quic->sendstate, req->quic->sendstate.acked.ranges[0].end + req->sendbuf->size);
    } else {
        assert(chunk.len != 0);
    }

    req->proceed_req.bytes_inflight = chunk.len;
    quicly_stream_sync_sendbuf(req->quic, 1);
    h2o_quic_schedule_timer(&req->conn->super.super);
    return 0;
}


// Source: http3client.c
// Lines 804-834
