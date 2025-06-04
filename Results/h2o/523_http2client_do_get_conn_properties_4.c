static void do_get_conn_properties(h2o_httpclient_t *_client, h2o_httpclient_conn_properties_t *properties)
{
    struct st_h2o_http2client_stream_t *stream = (void *)_client;
    h2o_httpclient_set_conn_properties_of_socket(stream->conn->super.sock, properties);
}


// Source: http2client.c
// Lines 1279-1283
