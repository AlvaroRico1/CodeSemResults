static int collect_req_status(h2o_req_t *req, void *_cbdata)
{
    struct st_collect_req_status_cbdata_t *cbdata = _cbdata;

    /* collect log */
    char buf[4096];
    size_t len = sizeof(buf);
    char *logline = h2o_log_request(cbdata->logconf, req, &len, buf);
    assert(len != 0);
    --len; /* omit trailing LF */

    /* append to buffer */
    if ((h2o_buffer_try_reserve(&cbdata->buffer, len + 3)).base == NULL) {
        if (logline != buf)
            free(logline);
        return -1;
    }
    memcpy(cbdata->buffer->bytes + cbdata->buffer->size, logline, len);
    cbdata->buffer->size += len;

    if (logline != buf)
        free(logline);

    return 0;
}


// Source: requests.c
// Lines 36-60
