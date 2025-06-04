static int decode_header(h2o_mem_pool_t *pool, void *_ctx, h2o_iovec_t **name, h2o_iovec_t *value, const uint8_t **src,
                         const uint8_t *src_end, const char **err_desc)
{
    struct st_h2o_qpack_decode_header_ctx_t *ctx = _ctx;
    unsigned soft_errors;

    switch (**src >> 4) {
    case 12:
    case 13:
    case 14:
    case 15: /* indexed static header field */ {
        const h2o_qpack_static_table_entry_t *entry;
        if ((entry = resolve_static(src, src_end, 6, err_desc)) == NULL)
            goto Fail;
        *name = (h2o_iovec_t *)&entry->name->buf;
        *value = entry->value;
        soft_errors = 0;
    } break;
    case 8:
    case 9:
    case 10:
    case 11: /* indexed dynamic header field */ {
        struct st_h2o_qpack_header_t *entry;
        if ((entry = resolve_dynamic(&ctx->qpack->table, ctx->base_index, src, src_end, 6, err_desc)) == NULL)
            goto Fail;
        h2o_mem_link_shared(pool, entry);
        *name = entry->name;
        *value = h2o_iovec_init(entry->value, entry->value_len);
        soft_errors = entry->soft_errors;
    } break;


// Source: qpack.c
// Lines 671-700
