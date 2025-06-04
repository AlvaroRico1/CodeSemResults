void h2o_httpclient__h1_on_connect(h2o_httpclient_t *_client, h2o_socket_t *sock, h2o_url_t *origin)
{
    struct st_h2o_http1client_t *client = (void *)_client;

    assert(!h2o_timer_is_linked(&client->super._timeout));

    setup_client(client, sock, origin);
    on_connection_ready(client);
}


// Source: http1client.c
// Lines 807-815
