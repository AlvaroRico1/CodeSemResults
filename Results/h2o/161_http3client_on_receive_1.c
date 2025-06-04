static void on_receive(quicly_stream_t *qs, size_t off, const void *input, size_t len)
{
    struct st_h2o_http3client_req_t *req = qs->data;
    size_t bytes_consumed;
    int err = 0;
    const char *err_desc = NULL;

    /* process the input, update stream-level receive buffer */
    if (req->recvbuf.stream->size == 0 && off == 0) {

        /* fast path; process the input directly, save the remaining bytes */
        const uint8_t *src = input;
        err = on_receive_process_bytes(req, &src, src + len, &err_desc);
        bytes_consumed = src - (const uint8_t *)input;
        if (bytes_consumed != len)
            h2o_buffer_append(&req->recvbuf.stream, src, len - bytes_consumed);
    } else {
        /* slow path; copy data to partial_frame */
        size_t size_required = off + len;
        if (req->recvbuf.stream->size < size_required) {
            h2o_buffer_reserve(&req->recvbuf.stream, size_required - req->recvbuf.stream->size);
            req->recvbuf.stream->size = size_required;
        }
        memcpy(req->recvbuf.stream->bytes + off, input, len);

        /* just return if no new data is available */
        size_t bytes_available = quicly_recvstate_bytes_available(&req->quic->recvstate);
        if (req->recvbuf.prev_bytes_available == bytes_available)
            return;

        /* process the bytes that have not been processed, update stream-level buffer */
        const uint8_t *src = (const uint8_t *)req->recvbuf.stream->bytes;
        err = on_receive_process_bytes(req, &src, (const uint8_t *)req->recvbuf.stream->bytes + bytes_available, &err_desc);
        bytes_consumed = src - (const uint8_t *)req->recvbuf.stream->bytes;
        h2o_buffer_consume(&req->recvbuf.stream, bytes_consumed);
    }


// Source: http3client.c
// Lines 643-678
