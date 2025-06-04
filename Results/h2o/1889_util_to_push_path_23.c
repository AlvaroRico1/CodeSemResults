static h2o_iovec_t to_push_path(h2o_mem_pool_t *pool, h2o_iovec_t url, h2o_iovec_t base_path, const h2o_url_scheme_t *input_scheme,
                                h2o_iovec_t input_authority, const h2o_url_scheme_t *base_scheme, h2o_iovec_t *base_authority,
                                int allow_cross_origin_push)
{
    h2o_url_t parsed, resolved;

    /* check the authority, and extract absolute path */
    if (h2o_url_parse_relative(url.base, url.len, &parsed) != 0)
        goto Invalid;

    /* fast-path for abspath form */
    if (base_scheme == NULL && parsed.scheme == NULL && parsed.authority.base == NULL && url.len != 0 && url.base[0] == '/') {
        return h2o_strdup(pool, url.base, url.len);
    }

    /* check scheme and authority if given URL contains either of the two, or if base is specified */
    h2o_url_t base = {input_scheme, input_authority, {NULL}, base_path, 65535};
    if (base_scheme != NULL) {
        base.scheme = base_scheme;
        base.authority = *base_authority;
    }
    h2o_url_resolve(pool, &base, &parsed, &resolved);
    if (input_scheme != resolved.scheme)
        goto Invalid;
    if (!allow_cross_origin_push &&
        !h2o_lcstris(input_authority.base, input_authority.len, resolved.authority.base, resolved.authority.len))
        goto Invalid;

    return resolved.path;

Invalid:
    return h2o_iovec_init(NULL, 0);
}


// Source: util.c
// Lines 596-628
