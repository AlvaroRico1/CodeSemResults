static void log_access(h2o_logger_t *_self, h2o_req_t *req)
{
    struct st_h2o_access_logger_t *self = (struct st_h2o_access_logger_t *)_self;
    h2o_access_log_filehandle_t *fh = self->fh;
    char *logline, buf[4096];
    size_t len;

    /* stringify */
    len = sizeof(buf);
    logline = h2o_log_request(fh->logconf, req, &len, buf);

    /* emit */
    write(fh->fd, logline, len);

    /* free memory */
    if (logline != buf)
        free(logline);
}


// Source: access_log.c
// Lines 44-61
