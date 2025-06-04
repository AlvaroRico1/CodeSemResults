static void ingress_unistream_on_receive_reset(quicly_stream_t *qs, int err)
{
    h2o_http3_conn_t *conn = *quicly_get_data(qs->conn);
    struct st_h2o_http3_ingress_unistream_t *stream = qs->data;

    stream->handle_input(conn, stream, NULL, NULL, 1);
}


// Source: common.c
// Lines 237-243
