h2o_iovec_t h2o_str_stripws(const char *s, size_t len)
{
    const char *end = s + len;

    while (s != end) {
        if (!is_ws(*s))
            break;
        ++s;
    }
    while (s != end) {
        if (!is_ws(end[-1]))
            break;
        --end;
    }
    return h2o_iovec_init(s, end - s);
}


// Source: string.c
// Lines 374-389
