static struct st_h2o_hpack_header_table_entry_t *header_table_add(h2o_hpack_header_table_t *table, size_t size_add,
                                                                  size_t max_num_entries)
{
    /* adjust the size */
    while (table->num_entries != 0 && table->hpack_size + size_add > table->hpack_capacity)
        header_table_evict_one(table);
    while (max_num_entries <= table->num_entries)
        header_table_evict_one(table);
    if (table->num_entries == 0) {
        assert(table->hpack_size == 0);
        if (size_add > table->hpack_capacity)
            return NULL;
    }
    table->hpack_size += size_add;

    /* grow the entries if full */
    if (table->num_entries == table->entry_capacity) {
        size_t new_capacity = table->num_entries * 2;
        if (new_capacity < 16)
            new_capacity = 16;
        struct st_h2o_hpack_header_table_entry_t *new_entries =
            h2o_mem_alloc(new_capacity * sizeof(struct st_h2o_hpack_header_table_entry_t));
        if (table->num_entries != 0) {
            size_t src_index = table->entry_start_index, dst_index = 0;
            do {
                new_entries[dst_index] = table->entries[src_index];
                ++dst_index;
                src_index = (src_index + 1) % table->entry_capacity;
            } while (dst_index != table->num_entries);
        }
        memset(new_entries + table->num_entries, 0, sizeof(*new_entries) * (new_capacity - table->num_entries));
        free(table->entries);
        table->entries = new_entries;
        table->entry_capacity = new_capacity;
        table->entry_start_index = 0;
    }


// Source: hpack.c
// Lines 254-289
