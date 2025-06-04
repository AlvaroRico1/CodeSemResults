int ptls_receive(ptls_t *tls, ptls_buffer_t *decryptbuf, const void *_input, size_t *inlen)
{
    const uint8_t *input = (const uint8_t *)_input, *const end = input + *inlen;
    size_t decryptbuf_orig_size = decryptbuf->off;
    int ret = 0;

    assert(tls->state >= PTLS_STATE_SERVER_EXPECT_END_OF_EARLY_DATA);

    /* loop until we decrypt some application data (or an error) */
    while (ret == 0 && input != end && decryptbuf_orig_size == decryptbuf->off) {
        size_t consumed = end - input;
        ret = handle_input(tls, NULL, decryptbuf, input, &consumed, NULL);
        input += consumed;

        switch (ret) {
        case 0:
            break;
        case PTLS_ERROR_IN_PROGRESS:
            ret = 0;
            break;
        case PTLS_ERROR_CLASS_PEER_ALERT + PTLS_ALERT_CLOSE_NOTIFY:
            /* TODO send close alert */
            break;
        default:
            if (PTLS_ERROR_GET_CLASS(ret) == PTLS_ERROR_CLASS_SELF_ALERT) {
                /* TODO send alert */
            }
            break;
        }
    }

    *inlen -= end - input;

    return ret;
}


// Source: picotls.c
// Lines 4886-4920
