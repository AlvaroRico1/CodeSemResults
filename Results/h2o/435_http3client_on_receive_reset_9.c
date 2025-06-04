static void on_receive_reset(quicly_stream_t *qs, int err)
{
    struct st_h2o_http3client_req_t *req = qs->data;

    notify_response_error(req, h2o_httpclient_error_io);
    close_stream(req, H2O_HTTP3_ERROR_REQUEST_CANCELLED);
    destroy_request(req);
}


// Source: http3client.c
// Lines 708-715
