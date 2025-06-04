static void do_get_conn_properties(h2o_httpclient_t *_client, h2o_httpclient_conn_properties_t *properties)
{
    struct st_h2o_http1client_t *client = (void *)_client;
    h2o_httpclient_set_conn_properties_of_socket(client->sock, properties);
}


// Source: http1client.c
// Lines 788-792
