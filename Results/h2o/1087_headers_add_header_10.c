static ssize_t add_header(h2o_mem_pool_t *pool, h2o_headers_t *headers, h2o_iovec_t *name, const char *orig_name, const char *value,
                          size_t value_len, h2o_header_flags_t flags)
{
    h2o_header_t *slot;

    h2o_vector_reserve(pool, headers, headers->size + 1);
    slot = headers->entries + headers->size++;

    slot->name = name;
    slot->value.base = (char *)value;
    slot->value.len = value_len;
    slot->orig_name = orig_name ? h2o_strdup(pool, orig_name, name->len).base : NULL;
    slot->flags = flags;
    return headers->size - 1;
}


// Source: headers.c
// Lines 26-40
