static void do_get_conn_properties(h2o_httpclient_t *_client, h2o_httpclient_conn_properties_t *properties)
{
    struct st_h2o_http3client_req_t *req = (void *)_client;
    ptls_t *tls;
    ptls_cipher_suite_t *cipher;

    if (req->quic != NULL && (tls = quicly_get_tls(req->quic->conn), (cipher = ptls_get_cipher(tls)) != NULL)) {
        properties->ssl.protocol_version = "TLSv1.3";
        properties->ssl.session_reused = ptls_is_psk_handshake(tls);
        properties->ssl.cipher = cipher->aead->name;
        properties->ssl.cipher_bits = (int)cipher->aead->key_size;
    } else {
        properties->ssl.protocol_version = NULL;
        properties->ssl.session_reused = -1;
        properties->ssl.cipher = NULL;
        properties->ssl.cipher_bits = 0;
    }
    properties->sock = NULL;
}


// Source: http3client.c
// Lines 778-796
