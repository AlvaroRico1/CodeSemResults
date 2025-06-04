static void handle_chunked_entity_read(struct st_h2o_http1_conn_t *conn)
{
    struct st_h2o_http1_chunked_entity_reader *reader = (void *)conn->_req_entity_reader;
    size_t bufsz;
    ssize_t ret;

    /* decode the incoming data */
    if ((bufsz = conn->sock->input->size) == 0)
        return;
    ret = phr_decode_chunked(&reader->decoder, conn->sock->input->bytes, &bufsz);
    if (ret != -1 && bufsz + conn->req.req_body_bytes_received >= conn->super.ctx->globalconf->max_request_entity_size) {
        entity_read_send_error_413(conn, "Request Entity Too Large", "request entity is too large");
        return;
    }
    if (ret < 0) {
        if (ret == -2) {
            /* incomplete */
            handle_one_body_fragment(conn, bufsz, conn->sock->input->size - bufsz, 0);
        } else {
            /* error */
            entity_read_send_error_400(conn, "Invalid Request", "broken chunked-encoding");
        }
    } else {
        /* complete */
        assert(bufsz + ret <= conn->sock->input->size);
        conn->sock->input->size = bufsz + ret;
        handle_one_body_fragment(conn, bufsz, 0, 1);
    }
}


// Source: http1.c
// Lines 266-294
