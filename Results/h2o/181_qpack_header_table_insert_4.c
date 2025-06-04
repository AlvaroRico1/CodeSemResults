static void header_table_insert(struct st_h2o_qpack_header_table_t *table, struct st_h2o_qpack_header_t *added)
{
    header_table_evict(table, added->name->len + added->value_len + HEADER_ENTRY_SIZE_OFFSET);

    if (table->last == table->buf_end) {
        size_t count = table->last - table->first, new_capacity = count <= 2 ? 4 : count * 2;
        if (new_capacity > table->buf_end - table->buf_start) {
            struct st_h2o_qpack_header_t **newbuf = h2o_mem_alloc(sizeof(*newbuf) * new_capacity);
            memcpy(newbuf, table->first, sizeof(*newbuf) * count);
            free(table->buf_start);
            table->buf_start = newbuf;
            table->first = newbuf;
            table->last = newbuf + count;
            table->buf_end = newbuf + new_capacity;
        } else {
            assert(table->buf_start != table->first);
            memmove(table->buf_start, table->first, sizeof(*table->buf_start) * count);
            table->first = table->buf_start;
            table->last = table->buf_start + count;
        }


// Source: qpack.c
// Lines 178-197
