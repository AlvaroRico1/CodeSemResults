int h2o_url_parse_relative(const char *url, size_t url_len, h2o_url_t *parsed)
{
    const char *url_end, *p;

    if (url_len == SIZE_MAX)
        url_len = strlen(url);
    url_end = url + url_len;

    /* obtain scheme and port number */
    if ((p = parse_scheme(url, url_end, &parsed->scheme)) == NULL) {
        parsed->scheme = NULL;
        p = url;
    }

    /* handle "//" */
    if (url_end - p >= 2 && p[0] == '/' && p[1] == '/')
        return parse_authority_and_path(p + 2, url_end, parsed);

    /* reset authority, host, port, and set path */
    parsed->authority = (h2o_iovec_t){NULL};
    parsed->host = (h2o_iovec_t){NULL};
    parsed->_port = 65535;
    parsed->path = h2o_iovec_init(p, url_end - p);

    return 0;
}


// Source: url.c
// Lines 267-292
