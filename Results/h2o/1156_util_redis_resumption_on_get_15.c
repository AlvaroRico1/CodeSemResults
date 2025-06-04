static void redis_resumption_on_get(redisReply *reply, void *_accept_data, const char *errstr)
{
    struct st_h2o_redis_resumption_accept_data_t *accept_data = _accept_data;
    accept_data->get_command = NULL;

    h2o_iovec_t session_data;
    if (reply != NULL && reply->type == REDIS_REPLY_STRING) {
        session_data = h2o_decode_base64url(NULL, reply->str, reply->len);
    } else {
        session_data = h2o_iovec_init(NULL, 0);
    }

    h2o_socket_ssl_resume_server_handshake(accept_data->super.sock, session_data);

    if (session_data.base != NULL)
        free(session_data.base);
}


// Source: util.c
// Lines 226-242
