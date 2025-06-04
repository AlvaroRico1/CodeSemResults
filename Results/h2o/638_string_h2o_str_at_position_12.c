int h2o_str_at_position(char *buf, const char *src, size_t src_len, int lineno, int column)
{
    const char *src_end = src + src_len;
    int i;

    /* find the line */
    if (lineno <= 0 || column <= 0)
        return -1;
    for (--lineno; lineno != 0; --lineno) {
        do {
            if (src == src_end)
                return -1;
        } while (*src++ != '\n');
    }

    /* adjust the starting column */
    while (column > 40) {
        if (src != src_end)
            ++src;
        --column;
    }

    /* emit */
    for (i = 1; i <= 76; ++i) {
        if (src == src_end || *src == '\n')
            break;
        *buf++ = *src++;
    }
    if (i < column)
        column = i;
    *buf++ = '\n';
    for (i = 1; i < column; ++i)
        *buf++ = ' ';
    *buf++ = '^';
    *buf++ = '\n';
    *buf = '\0';
    return 0;
}


// Source: string.c
// Lines 592-629
