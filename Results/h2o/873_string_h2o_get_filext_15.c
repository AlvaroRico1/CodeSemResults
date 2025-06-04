h2o_iovec_t h2o_get_filext(const char *path, size_t len)
{
    const char *end = path + len, *p = end;

    while (--p != path) {
        if (*p == '.') {
            return h2o_iovec_init(p + 1, end - (p + 1));
        } else if (*p == '/') {
            break;
        }
    }
    return h2o_iovec_init(NULL, 0);
}


// Source: string.c
// Lines 355-367
