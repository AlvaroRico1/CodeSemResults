static void discard_entry(quicly_sentmap_t *map, quicly_sentmap_iter_t *iter)
{
    assert(iter->p->acked != NULL);
    iter->p->acked = NULL;

    struct st_quicly_sent_block_t *block = *iter->ref;
    if (--block->num_entries == 0) {
        iter->ref = free_block(map, iter->ref);
        block = *iter->ref;
        iter->p = block->entries - 1;
        iter->count = block->num_entries + 1;
    }
}


// Source: sentmap.c
// Lines 71-83
