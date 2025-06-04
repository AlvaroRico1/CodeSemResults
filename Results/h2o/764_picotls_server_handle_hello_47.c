static int server_handle_hello(ptls_t *tls, ptls_message_emitter_t *emitter, ptls_iovec_t message,
                               ptls_handshake_properties_t *properties)
{
#define EMIT_SERVER_HELLO(sched, fill_rand, extensions)                                                                            \
    ptls_push_message(emitter, (sched), PTLS_HANDSHAKE_TYPE_SERVER_HELLO, {                                                        \
        ptls_buffer_push16(emitter->buf, 0x0303 /* legacy version */);                                                             \
        if ((ret = ptls_buffer_reserve(emitter->buf, PTLS_HELLO_RANDOM_SIZE)) != 0)                                                \
            goto Exit;                                                                                                             \
        do {                                                                                                                       \
            fill_rand                                                                                                              \
        } while (0);                                                                                                               \
        emitter->buf->off += PTLS_HELLO_RANDOM_SIZE;                                                                               \
        ptls_buffer_push_block(emitter->buf, 1,                                                                                    \
                               { ptls_buffer_pushv(emitter->buf, ch->legacy_session_id.base, ch->legacy_session_id.len); });       \
        ptls_buffer_push16(emitter->buf, tls->cipher_suite->id);                                                                   \
        ptls_buffer_push(emitter->buf, 0);                                                                                         \
        ptls_buffer_push_block(emitter->buf, 2, {                                                                                  \
            buffer_push_extension(emitter->buf, PTLS_EXTENSION_TYPE_SUPPORTED_VERSIONS,                                            \
                                  { ptls_buffer_push16(emitter->buf, ch->selected_version); });                                    \
            do {                                                                                                                   \
                extensions                                                                                                         \
            } while (0);                                                                                                           \
        });                                                                                                                        \
    });

#define EMIT_HELLO_RETRY_REQUEST(sched, negotiated_group, additional_extensions)                                                   \
    EMIT_SERVER_HELLO((sched), { memcpy(emitter->buf->base + emitter->buf->off, hello_retry_random, PTLS_HELLO_RANDOM_SIZE); },    \
                      {                                                                                                            \
                          ptls_key_exchange_algorithm_t *_negotiated_group = (negotiated_group);                                   \
                          if (_negotiated_group != NULL) {                                                                         \
                              buffer_push_extension(emitter->buf, PTLS_EXTENSION_TYPE_KEY_SHARE,                                   \
                                                    { ptls_buffer_push16(emitter->buf, _negotiated_group->id); });                 \
                          }                                                                                                        \
                          do {                                                                                                     \
                              additional_extensions                                                                                \
                          } while (0);                                                                                             \
                      })
    struct st_ptls_client_hello_t *ch;
    struct {
        ptls_key_exchange_algorithm_t *algorithm;
        ptls_iovec_t peer_key;
    } key_share = {NULL};


// Source: picotls.c
// Lines 3650-3691
