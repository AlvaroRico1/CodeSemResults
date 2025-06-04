static uint8_t *do_encode_header(h2o_hpack_header_table_t *header_table, uint8_t *dst, const h2o_iovec_t *name,
                                 const h2o_iovec_t *value, int dont_compress)
{
    int is_token = h2o_iovec_is_token(name);
    int name_index = is_token ? ((const h2o_token_t *)name)->flags.http2_static_table_name_index : 0;

    /* try to send as indexed */
    {
        size_t header_table_index = header_table->entry_start_index, n;
        for (n = header_table->num_entries; n != 0; --n) {
            struct st_h2o_hpack_header_table_entry_t *entry = header_table->entries + header_table_index;
            if (is_token) {
                if (name != entry->name)
                    goto Next;
            } else {
                if (!h2o_memis(name->base, name->len, entry->name->base, entry->name->len))
                    goto Next;
                if (name_index == 0)
                    name_index = (int)(header_table->num_entries - n + HEADER_TABLE_OFFSET);
            }
            /* name matched! */
            if (!h2o_memis(value->base, value->len, entry->value->base, entry->value->len))
                goto Next;
            /* name and value matched! */
            *dst = 0x80;
            dst = h2o_hpack_encode_int(dst, header_table->num_entries - n + HEADER_TABLE_OFFSET, 7);
            return dst;
        Next:
            ++header_table_index;
            if (header_table_index == header_table->entry_capacity)
                header_table_index = 0;
        }


// Source: hpack.c
// Lines 766-797
