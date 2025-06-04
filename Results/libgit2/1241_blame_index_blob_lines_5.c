static int index_blob_lines(git_blame *blame)
{
    const char *buf = blame->final_buf;
    size_t len = blame->final_buf_size;
    int num = 0, incomplete = 0, bol = 1;
    size_t *i;

    if (len && buf[len-1] != '\n')
        incomplete++; /* incomplete line at the end */
    while (len--) {
        if (bol) {
            i = git_array_alloc(blame->line_index);
            GIT_ERROR_CHECK_ALLOC(i);
            *i = buf - blame->final_buf;
            bol = 0;
        }
        if (*buf++ == '\n') {
            num++;
            bol = 1;
        }
    }
    i = git_array_alloc(blame->line_index);
    GIT_ERROR_CHECK_ALLOC(i);
    *i = buf - blame->final_buf;
    blame->num_lines = num + incomplete;
    return blame->num_lines;
}


// Source: blame.c
// Lines 274-300
