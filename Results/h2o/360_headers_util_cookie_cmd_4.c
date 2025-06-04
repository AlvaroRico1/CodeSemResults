static void cookie_cmd(h2o_mem_pool_t *pool, h2o_headers_t *headers, h2o_headers_command_t *cmd)
{
    ssize_t header_index;
    for (header_index = -1; (header_index = h2o_find_header(headers, H2O_TOKEN_COOKIE, header_index)) != -1;) {
        h2o_header_t *header = headers->entries + header_index;
        filter_cookie(pool, &header->value.base, &header->value.len, cmd);
        if (header->value.len == 0)
            h2o_delete_header(headers, header_index);
    }


// Source: headers_util.c
// Lines 70-78
