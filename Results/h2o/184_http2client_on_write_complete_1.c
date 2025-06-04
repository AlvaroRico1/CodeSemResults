static void on_write_complete(h2o_socket_t *sock, const char *err)
{
    struct st_h2o_http2client_conn_t *conn = sock->data;

    assert(conn->output.buf_in_flight != NULL);

    h2o_timer_unlink(&conn->io_timeout);

    /* close by error if necessary */
    if (err != NULL) {
        call_stream_callbacks_with_error(conn, h2o_httpclient_error_io);
        close_connection_now(conn);
        return;
    }

    if (close_connection_if_necessary(conn))
        return;

    /* unlink timeouts of streams that has finished sending requests */
    while (!h2o_linklist_is_empty(&conn->output.sent_streams)) {
        h2o_linklist_t *link = conn->output.sent_streams.next;
        struct st_h2o_http2client_stream_t *stream =
            H2O_STRUCT_FROM_MEMBER(struct st_h2o_http2client_stream_t, output.sending_link, link);
        h2o_linklist_unlink(link);

        if (stream->streaming.proceed_req != NULL && stream->streaming.inflight) {
            stream->streaming.inflight = 0;
            stream->streaming.proceed_req(&stream->super, NULL);
        }

        if (stream->streaming.proceed_req == NULL || stream->streaming.done) {
            stream->state.req = STREAM_STATE_CLOSED;
            h2o_timer_link(stream->super.ctx->loop, stream->super.ctx->first_byte_timeout, &stream->super._timeout);
        }
    }

    /* reset the other buffer */
    h2o_buffer_dispose(&conn->output.buf_in_flight);

#if !H2O_USE_LIBUV
    if (conn->state == H2O_HTTP2CLIENT_CONN_STATE_OPEN) {
        if (conn->output.buf->size != 0 || !h2o_linklist_is_empty(&conn->output.sending_streams))
            h2o_socket_notify_write(sock, on_notify_write);
        return;
    }
#endif

    /* write more, if possible */
    do_emit_writereq(conn);
    close_connection_if_necessary(conn);
}


// Source: http2client.c
// Lines 1075-1125
