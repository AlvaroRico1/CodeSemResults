static void load_digest(h2o_cache_digests_t **digests, const char *gcs_base64, size_t gcs_base64_len, int with_validators,
                        int complete)
{
    h2o_cache_digests_frame_t frame = {{NULL}};
    h2o_iovec_t gcs_bin;
    struct st_golombset_decode_t ctx = {NULL};
    uint64_t nbits, pbits;

    /* decode base64 */
    if ((gcs_bin = h2o_decode_base64url(NULL, gcs_base64, gcs_base64_len)).base == NULL)
        goto Exit;

    /* prepare GCS context */
    ctx.src = (void *)(gcs_bin.base - 1);
    ctx.src_max = (void *)(gcs_bin.base + gcs_bin.len);
    ctx.src_shift = 0;

    /* decode nbits and pbits */
    if (golombset_decode_bits(&ctx, 5, &nbits) != 0 || golombset_decode_bits(&ctx, 5, &pbits) != 0)
        goto Exit;
    frame.capacity_bits = (unsigned)(nbits + pbits);

    /* decode the values */
    uint64_t value = UINT64_MAX, decoded;
    while (golombset_decode_value(&ctx, (unsigned)pbits, &decoded) == 0) {
        value += decoded + 1;
        if (value >= (uint64_t)1 << frame.capacity_bits)
            goto Exit;
        h2o_vector_reserve(NULL, &frame.keys, frame.keys.size + 1);
        frame.keys.entries[frame.keys.size++] = value;
    }

    /* store the result */
    if (*digests == NULL) {
        *digests = h2o_mem_alloc(sizeof(**digests));
        **digests = (h2o_cache_digests_t){{{NULL}}};
    }
    h2o_cache_digests_frame_vector_t *target = with_validators ? &(*digests)->fresh.url_and_etag : &(*digests)->fresh.url_only;
    h2o_vector_reserve(NULL, target, target->size + 1);
    target->entries[target->size++] = frame;
    frame = (h2o_cache_digests_frame_t){{NULL}};
    (*digests)->fresh.complete = complete;

Exit:
    free(frame.keys.entries);
    free(gcs_bin.base);
}


// Source: cache_digests.c
// Lines 54-100
