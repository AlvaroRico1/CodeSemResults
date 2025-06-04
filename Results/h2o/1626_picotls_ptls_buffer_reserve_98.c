int ptls_buffer_reserve(ptls_buffer_t *buf, size_t delta)
{
    if (buf->base == NULL)
        return PTLS_ERROR_NO_MEMORY;

    if (PTLS_MEMORY_DEBUG || buf->capacity < buf->off + delta) {
        uint8_t *newp;
        size_t new_capacity = buf->capacity;
        if (new_capacity < 1024)
            new_capacity = 1024;
        while (new_capacity < buf->off + delta) {
            new_capacity *= 2;
        }
        if ((newp = malloc(new_capacity)) == NULL)
            return PTLS_ERROR_NO_MEMORY;
        memcpy(newp, buf->base, buf->off);
        ptls_buffer__release_memory(buf);
        buf->base = newp;
        buf->capacity = new_capacity;
        buf->is_allocated = 1;
    }


// Source: picotls.c
// Lines 516-536
