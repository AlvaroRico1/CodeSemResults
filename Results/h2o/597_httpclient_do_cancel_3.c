static void do_cancel(h2o_httpclient_t *_client)
{
    h2o_httpclient_t *client = (void *)_client;
    close_client(client);
}


// Source: httpclient.c
// Lines 79-83
