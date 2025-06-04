void quicly_sentmap_dispose(quicly_sentmap_t *map)
{
    struct st_quicly_sent_block_t *block;

    while ((block = map->head) != NULL) {
        map->head = block->next;
        free(block);
    }
}


// Source: sentmap.c
// Lines 85-93
