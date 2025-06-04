static size_t calc_headers_capacity(const h2o_header_t *headers, size_t num_headers)
{
    const h2o_header_t *header;
    size_t capacity = 0;
    for (header = headers; num_headers != 0; ++header, --num_headers)
        capacity += calc_capacity(header->name->len, header->value.len);
    return capacity;
}


// Source: hpack.c
// Lines 911-918
