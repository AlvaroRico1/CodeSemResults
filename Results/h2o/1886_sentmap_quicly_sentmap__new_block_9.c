struct st_quicly_sent_block_t *quicly_sentmap__new_block(quicly_sentmap_t *map)
{
    struct st_quicly_sent_block_t *block;

    if ((block = malloc(sizeof(*block))) == NULL)
        return NULL;

    block->next = NULL;
    block->num_entries = 0;
    block->next_insert_at = 0;
    if (map->tail != NULL) {
        map->tail->next = block;
        map->tail = block;
    } else {
        map->head = map->tail = block;
    }

    return block;
}


// Source: sentmap.c
// Lines 105-123
