static void on_send_stop(quicly_stream_t *qs, int err)
{
    struct st_h2o_http3client_req_t *req;

    if ((req = qs->data) == NULL)
        return;

    if (!quicly_sendstate_transfer_complete(&req->quic->sendstate))
        quicly_reset_stream(req->quic, err);

    if (req->proceed_req.bytes_inflight != SIZE_MAX)
        call_proceed_req(req, h2o_httpclient_error_io /* TODO better error code? */);

    if (!quicly_recvstate_transfer_complete(&req->quic->recvstate)) {
        quicly_request_stop(req->quic, H2O_HTTP3_ERROR_REQUEST_CANCELLED);
        notify_response_error(req, h2o_httpclient_error_io);
    }
    detach_stream(req);
    destroy_request(req);
}


// Source: http3client.c
// Lines 605-624
