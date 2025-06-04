static void on_stream_destroy(quicly_stream_t *qs, int err)
{
    struct st_h2o_http3client_req_t *req;

    if ((req = qs->data) == NULL)
        return;
    notify_response_error(req, h2o_httpclient_error_io);
    detach_stream(req);
    destroy_request(req);
}


// Source: http3client.c
// Lines 570-579
