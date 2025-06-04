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
    case 5:
    case 7: /* literal header field with static name reference */ {
        const h2o_qpack_static_table_entry_t *entry;
        if ((entry = resolve_static(src, src_end, 4, err_desc)) == NULL)
            goto Fail;
        *name = (h2o_iovec_t *)&entry->name->buf;
        soft_errors = 0;
        if ((*value = decode_header_value_literal(pool, &soft_errors, src, src_end, err_desc)).base == NULL)
            goto Fail;
    } break;
    case 4:
    case 6: /* literal header field with dynamic name reference */ {
        struct st_h2o_qpack_header_t *entry;
        if ((entry = resolve_dynamic(&ctx->qpack->table, ctx->base_index, src, src_end, 4, err_desc)) == NULL)
            goto Fail;
        h2o_mem_link_shared(pool, entry);
        *name = entry->name;
        soft_errors = (entry->soft_errors) & H2O_HPACK_SOFT_ERROR_BIT_INVALID_NAME;
        if ((*value = decode_header_value_literal(pool, &soft_errors, src, src_end, err_desc)).base == NULL)
            goto Fail;
    } break;
    case 2:
    case 3: /* literal header field without name reference */ {
        soft_errors = 0;
        if ((*name = decode_header_name_literal(pool, &soft_errors, src, src_end, 3, err_desc)) == NULL)
            goto Fail;
        if ((*value = decode_header_value_literal(pool, &soft_errors, src, src_end, err_desc)).base == NULL)
            goto Fail;
    } break;
    case 1: /* indexed header field with post-base index */ {
        struct st_h2o_qpack_header_t *entry;
        if ((entry = resolve_dynamic_postbase(&ctx->qpack->table, ctx->base_index, src, src_end, 4, err_desc)) == NULL)
            goto Fail;
        h2o_mem_link_shared(pool, entry);
        *name = entry->name;
        *value = h2o_iovec_init(entry->value, entry->value_len);
        soft_errors = entry->soft_errors;
    } break;
    case 0: /* literal header field with post-base name reference */ {
        struct st_h2o_qpack_header_t *entry;
        if ((entry = resolve_dynamic_postbase(&ctx->qpack->table, ctx->base_index, src, src_end, 3, err_desc)) == NULL)
            goto Fail;
        h2o_mem_link_shared(pool, entry);
        *name = entry->name;
        soft_errors = (entry->soft_errors) & H2O_HPACK_SOFT_ERROR_BIT_INVALID_NAME;
        if ((*value = decode_header_value_literal(pool, &soft_errors, src, src_end, err_desc)).base == NULL)
            goto Fail;
    } break;
    default:
        h2o_fatal("unreachable");
        soft_errors = 0;
        break;
    }

    if (soft_errors != 0) {
        *err_desc = (soft_errors & H2O_HPACK_SOFT_ERROR_BIT_INVALID_NAME) != 0
                        ? h2o_hpack_soft_err_found_invalid_char_in_header_name
                        : h2o_hpack_soft_err_found_invalid_char_in_header_value;
        return H2O_HTTP2_ERROR_INVALID_HEADER_CHAR;
    }
    return 0;
Fail:
    return H2O_HTTP3_ERROR_QPACK_DECOMPRESSION_FAILED;
}


// Source: qpack.c
// Lines 671-764
