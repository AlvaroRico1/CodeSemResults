static void on_write_complete(h2o_socket_t *sock, const char *err)
{
    h2o_http2_conn_t *conn = sock->data;

    assert(conn->_write.buf_in_flight != NULL);

    /* close by error if necessary */
    if (err != NULL) {
        conn->super.ctx->http2.events.write_closed++;
        close_connection_now(conn);
        return;
    }

    /* reset the other memory pool */
    h2o_buffer_dispose(&conn->_write.buf_in_flight);
    assert(conn->_write.buf_in_flight == NULL);

    /* call the proceed callback of the streams that have been flushed (while unlinking them from the list) */
    if (conn->state < H2O_HTTP2_CONN_STATE_IS_CLOSING) {
        while (!h2o_linklist_is_empty(&conn->_write.streams_to_proceed)) {
            h2o_http2_stream_t *stream = H2O_STRUCT_FROM_MEMBER(h2o_http2_stream_t, _link, conn->_write.streams_to_proceed.next);
            assert(!h2o_http2_stream_has_pending_data(stream));
            h2o_linklist_unlink(&stream->_link);
            h2o_http2_stream_proceed(conn, stream);
        }
    }

    /* update the timeout now that the states have been updated */
    update_idle_timeout(conn);

    /* cancel the write callback if scheduled (as the generator may have scheduled a write just before this function gets called) */
    if (h2o_timer_is_linked(&conn->_write.timeout_entry))
        h2o_timer_unlink(&conn->_write.timeout_entry);

    if (conn->state < H2O_HTTP2_CONN_STATE_IS_CLOSING) {
        if (!h2o_socket_is_reading(conn->sock) && bytes_in_buf(conn) < H2O_HTTP2_DEFAULT_OUTBUF_SOFT_MAX_SIZE)
            h2o_socket_read_start(conn->sock, on_read);
    }

#if !H2O_USE_LIBUV
    if (conn->state == H2O_HTTP2_CONN_STATE_OPEN) {
        if (conn->_write.buf->size != 0 || h2o_http2_scheduler_is_active(&conn->scheduler))
            h2o_socket_notify_write(sock, on_notify_write);
        return;
    }
#endif

    /* write more, if possible */
    do_emit_writereq(conn);
}


// Source: connection.c
// Lines 1423-1472
