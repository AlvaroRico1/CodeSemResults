static void on_send_emit(quicly_stream_t *qs, size_t off, void *dst, size_t *len, int *wrote_all)
{
    struct st_h2o_http3client_req_t *req = qs->data;

    if (*len >= req->sendbuf->size - off) {
        *len = req->sendbuf->size - off;
        *wrote_all = 1;
    } else {
        *wrote_all = 0;
    }
    memcpy(dst, req->sendbuf->bytes + off, *len);

    if (*wrote_all && req->proceed_req.bytes_inflight != SIZE_MAX)
        call_proceed_req(req, NULL);
}


// Source: http3client.c
// Lines 589-603
