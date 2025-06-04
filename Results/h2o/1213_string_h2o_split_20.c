void h2o_split(h2o_mem_pool_t *pool, h2o_iovec_vector_t *list, h2o_iovec_t str, const char needle)
{
    const char *p = str.base, *end = str.base + str.len, *found;

    while (p < end && (found = memchr(p, needle, end - p)) != NULL) {
        h2o_vector_reserve(pool, list, list->size + 1);
        list->entries[list->size++] = h2o_strdup(pool, p, found - p);
        p = found + 1;
    }
    h2o_vector_reserve(pool, list, list->size + 1);
    list->entries[list->size++] = h2o_strdup(pool, p, end - p);
}


// Source: string.c
// Lines 579-590
