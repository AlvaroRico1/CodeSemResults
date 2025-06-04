static int lookup(h2o_cache_digests_frame_vector_t *vector, const char *url, size_t url_len, const char *etag, size_t etag_len,
                  int is_fresh, int is_complete)
{
    if (vector->size != 0) {
        uint64_t hash = calc_hash(url, url_len, etag, etag_len);
        size_t i = 0;
        do {
            h2o_cache_digests_frame_t *frame = vector->entries + i;
            uint64_t key = hash >> (64 - frame->capacity_bits);
            if (frame->keys.entries != NULL &&
                bsearch(&key, frame->keys.entries, frame->keys.size, sizeof(frame->keys.entries[0]), cmp_key) != NULL)
                return is_fresh ? H2O_CACHE_DIGESTS_STATE_FRESH : H2O_CACHE_DIGESTS_STATE_STALE;
        } while (++i != vector->size);


// Source: cache_digests.c
// Lines 174-186
