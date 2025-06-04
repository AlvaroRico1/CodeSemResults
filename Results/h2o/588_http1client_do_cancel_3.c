static void do_cancel(h2o_httpclient_t *_client)
{
    struct st_h2o_http1client_t *client = (struct st_h2o_http1client_t *)_client;
    client->_do_keepalive = 0;
    close_client(client);
}


// Source: http1client.c
// Lines 766-771
