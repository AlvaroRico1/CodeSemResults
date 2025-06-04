static void on_send_shift(quicly_stream_t *qs, size_t delta)
{
    struct st_h2o_http3client_req_t *req = qs->data;

    assert(req != NULL);
    h2o_buffer_consume(&req->sendbuf, delta);
}


// Source: http3client.c
// Lines 581-587
