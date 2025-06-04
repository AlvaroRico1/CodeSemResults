static char *expand_line_buf(char *line, size_t *cur_size, size_t required, int should_realloc)
{
    size_t new_size = *cur_size;

    /* determine the new size */
    do {
        new_size *= 2;
    } while (new_size < required);

    /* reallocate */
    if (!should_realloc) {
        char *newpt = h2o_mem_alloc(new_size);
        memcpy(newpt, line, *cur_size);
        line = newpt;
    } else {
        line = h2o_mem_realloc(line, new_size);
    }


// Source: logconf.c
// Lines 511-527
