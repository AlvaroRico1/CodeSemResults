ssize_t h2o_find_header_by_str(const h2o_headers_t *headers, const char *name, size_t name_len, ssize_t cursor)
{
    for (++cursor; cursor < headers->size; ++cursor) {
        h2o_header_t *t = headers->entries + cursor;
        if (h2o_memis(t->name->base, t->name->len, name, name_len)) {
            return cursor;
        }
    }


// Source: headers.c
// Lines 60-67
