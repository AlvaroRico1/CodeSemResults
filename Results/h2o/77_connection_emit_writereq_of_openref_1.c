static int emit_writereq_of_openref(h2o_http2_scheduler_openref_t *ref, int *still_is_active, void *cb_arg)
{
    h2o_http2_conn_t *conn = cb_arg;
    h2o_http2_stream_t *stream = H2O_STRUCT_FROM_MEMBER(h2o_http2_stream_t, _scheduler, ref);

    assert(h2o_http2_stream_has_pending_data(stream) || stream->state >= H2O_HTTP2_STREAM_STATE_SEND_BODY_IS_FINAL);

    *still_is_active = 0;

    h2o_http2_stream_send_pending_data(conn, stream);
    if (h2o_http2_stream_has_pending_data(stream) || stream->state == H2O_HTTP2_STREAM_STATE_SEND_BODY_IS_FINAL) {
        if (h2o_http2_window_get_avail(&stream->output_window) <= 0) {
            /* is blocked */
        } else {
            *still_is_active = 1;
        }
    } else {
        if (stream->state == H2O_HTTP2_STREAM_STATE_END_STREAM && stream->req.send_server_timing) {
            h2o_header_t trailers[1];
            size_t num_trailers = 0;
            h2o_iovec_t server_timing;
            if ((server_timing = h2o_build_server_timing_trailer(&stream->req, NULL, 0, NULL, 0)).len != 0) {
                static const h2o_iovec_t name = {H2O_STRLIT("server-timing")};
                trailers[num_trailers++] = (h2o_header_t){(h2o_iovec_t *)&name, NULL, server_timing};
            }
            h2o_hpack_flatten_trailers(&conn->_write.buf, &conn->_output_header_table, conn->peer_settings.header_table_size,
                                       stream->stream_id, conn->peer_settings.max_frame_size, trailers, num_trailers);
        }
        h2o_linklist_insert(&conn->_write.streams_to_proceed, &stream->_link);
    }

    return h2o_http2_conn_get_buffer_window(conn) > 0 ? 0 : -1;
}


// Source: connection.c
// Lines 1474-1506
