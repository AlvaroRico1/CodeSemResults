void on_stream_destroy(quicly_stream_t *qs, int err)
{
    struct st_h2o_http3_server_stream_t *stream = qs->data;
    struct st_h2o_http3_server_conn_t *conn = get_conn(stream);

    --*get_state_counter(conn, stream->state);

    req_scheduler_deactivate(&conn->scheduler.reqs, &stream->scheduler);

    if (h2o_linklist_is_linked(&stream->link))
        h2o_linklist_unlink(&stream->link);
    if (stream->state != H2O_HTTP3_SERVER_STREAM_STATE_CLOSE_WAIT)
        pre_dispose_request(stream);
    if (!stream->req_disposed)
        h2o_dispose_request(&stream->req);
    /* in case the stream is destroyed before the buffer is fully consumed */
    h2o_buffer_dispose(&stream->recvbuf.buf);

    free(stream);
}


// Source: server.c
// Lines 661-680
