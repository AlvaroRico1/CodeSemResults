static int insert_at(quicly_ranges_t *ranges, uint64_t start, uint64_t end, size_t slot)
{
    if (ranges->num_ranges == ranges->capacity) {
        size_t new_capacity = ranges->capacity < 4 ? 4 : ranges->capacity * 2;
        quicly_range_t *new_ranges = malloc(new_capacity * sizeof(*new_ranges));
        if (new_ranges == NULL)
            return PTLS_ERROR_NO_MEMORY;
        COPY(new_ranges, ranges->ranges, slot);
        COPY(new_ranges + slot + 1, ranges->ranges + slot, ranges->num_ranges - slot);
        if (ranges->ranges != &ranges->_initial)
            free(ranges->ranges);
        ranges->ranges = new_ranges;
        ranges->capacity = new_capacity;
    } else {
        MOVE(ranges->ranges + slot + 1, ranges->ranges + slot, ranges->num_ranges - slot);
    }


// Source: ranges.c
// Lines 42-57
