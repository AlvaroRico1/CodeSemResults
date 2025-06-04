int quicly_receive(quicly_conn_t *conn, struct sockaddr *dest_addr, struct sockaddr *src_addr, quicly_decoded_packet_t *packet)
{
    ptls_cipher_context_t *header_protection;
    struct {
        int (*cb)(void *, uint64_t, quicly_decoded_packet_t *, size_t, size_t *);
        void *ctx;
    } aead;


// Source: quicly.c
// Lines 5792-5798
