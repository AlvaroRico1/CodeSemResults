static size_t *process_range(h2o_mem_pool_t *pool, h2o_iovec_t *range_value, size_t file_size, size_t *ret)
{
#define CHECK_EOF()                                                                                                                \
    if (buf == buf_end)                                                                                                            \
        return NULL;

#define CHECK_OVERFLOW(range)                                                                                                      \
    if (range == SIZE_MAX)                                                                                                         \
        return NULL;

    size_t range_start = SIZE_MAX, range_count = 0;
    char *buf = range_value->base, *buf_end = buf + range_value->len;
    int needs_comma = 0;
    H2O_VECTOR(size_t) ranges = {NULL};

    if (range_value->len < 6 || memcmp(buf, "bytes=", 6) != 0)
        return NULL;

    buf += 6;
    CHECK_EOF();

    /* most range requests contain only one range */
    do {
        while (1) {
            if (*buf != ',') {
                if (needs_comma)
                    return NULL;
                break;
            }
            needs_comma = 0;
            buf++;
            while (H2O_UNLIKELY(*buf == ' ') || H2O_UNLIKELY(*buf == '\t')) {
                buf++;
                CHECK_EOF();
            }
        }
        if (H2O_UNLIKELY(buf == buf_end))
            break;
        if (H2O_LIKELY((range_start = h2o_strtosizefwd(&buf, buf_end - buf)) != SIZE_MAX)) {
            CHECK_EOF();
            if (*buf++ != '-')
                return NULL;
            range_count = h2o_strtosizefwd(&buf, buf_end - buf);
            if (H2O_UNLIKELY(range_start >= file_size)) {
                range_start = SIZE_MAX;
            } else if (H2O_LIKELY(range_count != SIZE_MAX)) {
                if (H2O_UNLIKELY(range_count > file_size - 1))
                    range_count = file_size - 1;
                if (H2O_LIKELY(range_start <= range_count))
                    range_count -= range_start - 1;
                else
                    range_start = SIZE_MAX;
            } else {
                range_count = file_size - range_start;
            }
        } else if (H2O_LIKELY(*buf++ == '-')) {
            CHECK_EOF();
            range_count = h2o_strtosizefwd(&buf, buf_end - buf);
            if (H2O_UNLIKELY(range_count == SIZE_MAX))
                return NULL;
            if (H2O_LIKELY(range_count != 0)) {
                if (H2O_UNLIKELY(range_count > file_size))
                    range_count = file_size;
                range_start = file_size - range_count;
            } else {
                range_start = SIZE_MAX;
            }
        } else {
            return NULL;
        }

        if (H2O_LIKELY(range_start != SIZE_MAX)) {
            h2o_vector_reserve(pool, &ranges, ranges.size + 2);
            ranges.entries[ranges.size++] = range_start;
            ranges.entries[ranges.size++] = range_count;
        }
        if (buf != buf_end)
            while (H2O_UNLIKELY(*buf == ' ') || H2O_UNLIKELY(*buf == '\t')) {
                buf++;
                CHECK_EOF();
            }
        needs_comma = 1;
    } while (H2O_UNLIKELY(buf < buf_end));
    *ret = ranges.size / 2;
    return ranges.entries;
#undef CHECK_EOF
#undef CHECK_OVERFLOW
}


// Source: file.c
// Lines 450-537
