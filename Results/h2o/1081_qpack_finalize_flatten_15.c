static h2o_iovec_t finalize_flatten(struct st_h2o_qpack_flatten_context_t *ctx)
{
    if (ctx->largest_ref == 0) {
        ctx->base_index = 0;
    } else {
        int is_blocking = 0;
        /* adjust largest reference to achieve more compact representation on wire without risking blocking */
        if (ctx->largest_ref < ctx->qpack->largest_known_received) {
            ctx->largest_ref = ctx->qpack->largest_known_received;
        } else if (ctx->largest_ref > ctx->qpack->largest_known_received) {
            assert(ctx->qpack->num_blocked < ctx->qpack->max_blocked);
            ++ctx->qpack->num_blocked;
            is_blocking = 1;
        }
        /* mark as inflight */
        h2o_vector_reserve(NULL, &ctx->qpack->inflight, ctx->qpack->inflight.size + 1);
        ctx->qpack->inflight.entries[ctx->qpack->inflight.size++] =
            (struct st_h2o_qpack_blocked_streams_t){ctx->stream_id, ctx->largest_ref, {{is_blocking}}};
    }

    size_t start_off = PREFIX_CAPACITY;

    { /* prepend largest ref and delta base index */
        uint8_t buf[H2O_HPACK_ENCODE_INT_MAX_LENGTH * 2], *p = buf;
        /* largest_ref */
        *p = 0;
        p = h2o_hpack_encode_int(p, ctx->largest_ref != 0 ? ctx->largest_ref + 1 : 0, 8);
        /* delta base index */
        if (ctx->largest_ref <= ctx->base_index) {
            *p = 0;
            p = h2o_hpack_encode_int(p, ctx->base_index - ctx->largest_ref, 7);
        } else {
            *p = 0x80;
            p = h2o_hpack_encode_int(p, ctx->largest_ref - ctx->base_index - 1, 7);
        }
        memcpy(ctx->headers_buf.entries + start_off - (p - buf), buf, p - buf);
        start_off -= p - buf;
    }


// Source: qpack.c
// Lines 1240-1277
