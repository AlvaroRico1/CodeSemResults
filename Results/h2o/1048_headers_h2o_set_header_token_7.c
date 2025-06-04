ssize_t h2o_set_header_token(h2o_mem_pool_t *pool, h2o_headers_t *headers, const h2o_token_t *token, const char *value,
                             size_t value_len)
{
    ssize_t found = -1;
    size_t i;
    for (i = 0; i != headers->size; ++i) {
        if (headers->entries[i].name == &token->buf) {
            if (h2o_contains_token(headers->entries[i].value.base, headers->entries[i].value.len, value, value_len, ','))
                return -1;
            found = i;
        }
    }
    if (found != -1) {
        h2o_header_t *dest = headers->entries + found;
        dest->value = h2o_concat(pool, dest->value, h2o_iovec_init(H2O_STRLIT(", ")), h2o_iovec_init(value, value_len));
        return found;
    } else {
        return h2o_add_header(pool, headers, token, NULL, value, value_len);
    }


// Source: headers.c
// Lines 128-146
