static h2o_iovec_t rebuild_path(h2o_mem_pool_t *pool, const char *src, size_t src_len, size_t *query_at, size_t **norm_indexes)
{
    char *dst;
    size_t src_off = 0, dst_off = 0, last_slash, rewind;

    { /* locate '?', and set len to the end of input path */
        const char *q = memchr(src, '?', src_len);
        if (q != NULL) {
            src_len = *query_at = q - src;
        } else {
            *query_at = SIZE_MAX;
        }
    }


// Source: url.c
// Lines 64-76
