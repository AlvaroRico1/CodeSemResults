static void memcached_resumption_on_get(h2o_iovec_t session_data, void *_accept_data)
{
    struct st_h2o_memcached_resumption_accept_data_t *accept_data = _accept_data;
    accept_data->get_req = NULL;
    h2o_socket_ssl_resume_server_handshake(accept_data->super.sock, session_data);
}


// Source: util.c
// Lines 138-143
