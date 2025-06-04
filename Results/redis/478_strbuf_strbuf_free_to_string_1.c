char *strbuf_free_to_string(strbuf_t *s, int *len)
{
    char *buf;

    debug_stats(s);

    strbuf_ensure_null(s);

    buf = s->buf;
    if (len)
        *len = s->length;

    if (s->dynamic)
        free(s);

    return buf;
}


// Source: strbuf.c
// Lines 116-132
