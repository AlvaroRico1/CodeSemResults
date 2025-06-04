static void finalostream_send_informational(h2o_ostream_t *self, h2o_req_t *req)
{
    h2o_http2_stream_t *stream = H2O_STRUCT_FROM_MEMBER(h2o_http2_stream_t, _ostr_final, self);
    h2o_http2_conn_t *conn = (h2o_http2_conn_t *)req->conn;

    h2o_hpack_flatten_response(&conn->_write.buf, &conn->_output_header_table, conn->peer_settings.header_table_size,
                               stream->stream_id, conn->peer_settings.max_frame_size, req->res.status, req->res.headers.entries,
                               req->res.headers.size, NULL, SIZE_MAX);
    h2o_http2_conn_request_write(conn);
}


// Source: stream.c
// Lines 389-398
