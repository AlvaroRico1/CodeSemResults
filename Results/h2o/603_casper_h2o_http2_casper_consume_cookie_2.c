void h2o_http2_casper_consume_cookie(h2o_http2_casper_t *casper, const char *cookie, size_t cookie_len)
{
    h2o_iovec_t binary = {NULL};
    uint64_t tiny_keys_buf[128], *keys = tiny_keys_buf;

    /* check the name of the cookie */
    if (!(cookie_len > sizeof(COOKIE_NAME "=") - 1 && memcmp(cookie, H2O_STRLIT(COOKIE_NAME "=")) == 0))
        goto Exit;

    /* base64 decode */
    if ((binary = h2o_decode_base64url(NULL, cookie + sizeof(COOKIE_NAME "=") - 1, cookie_len - (sizeof(COOKIE_NAME "=") - 1)))
            .base == NULL)
        goto Exit;

    /* decode GCS, either using tiny_keys_buf or using heap */
    size_t capacity = sizeof(tiny_keys_buf) / sizeof(tiny_keys_buf[0]), num_keys;
    while (num_keys = capacity, golombset_decode(casper->remainder_bits, binary.base, binary.len, keys, &num_keys) != 0) {
        if (keys != tiny_keys_buf) {
            free(keys);
            keys = tiny_keys_buf; /* reset to something that would not trigger call to free(3) */
        }
        if (capacity >= (size_t)1 << casper->capacity_bits)
            goto Exit;
        capacity *= 2;
        keys = h2o_mem_alloc(capacity * sizeof(*keys));
    }

    /* copy or merge the entries */
    if (num_keys == 0) {
        /* nothing to do */
    } else if (casper->keys.size == 0) {
        h2o_vector_reserve(NULL, &casper->keys, num_keys);
        memcpy(casper->keys.entries, keys, num_keys * sizeof(*keys));
        casper->keys.size = num_keys;
    } else {
        uint64_t *orig_keys = casper->keys.entries;
        size_t num_orig_keys = casper->keys.size, orig_index = 0, new_index = 0;
        memset(&casper->keys, 0, sizeof(casper->keys));
        h2o_vector_reserve(NULL, &casper->keys, num_keys + num_orig_keys);
        do {
            if (orig_keys[orig_index] < keys[new_index]) {
                casper->keys.entries[casper->keys.size++] = orig_keys[orig_index++];
            } else if (orig_keys[orig_index] > keys[new_index]) {
                casper->keys.entries[casper->keys.size++] = keys[new_index++];
            } else {
                casper->keys.entries[casper->keys.size++] = orig_keys[orig_index];
                ++orig_index;
                ++new_index;
            }
        } while (orig_index != num_orig_keys && new_index != num_keys);
        if (orig_index != num_orig_keys) {
            do {
                casper->keys.entries[casper->keys.size++] = orig_keys[orig_index++];
            } while (orig_index != num_orig_keys);
        } else if (new_index != num_keys) {
            do {
                casper->keys.entries[casper->keys.size++] = keys[new_index++];
            } while (new_index != num_keys);
        }
        free(orig_keys);
    }

Exit:
    if (keys != tiny_keys_buf)
        free(keys);
    free(binary.base);
}


// Source: casper.c
// Lines 100-166
