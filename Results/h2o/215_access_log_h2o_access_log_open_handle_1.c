h2o_access_log_filehandle_t *h2o_access_log_open_handle(const char *path, const char *fmt, int escape)
{
    h2o_logconf_t *logconf;
    int fd;
    h2o_access_log_filehandle_t *fh;
    char errbuf[256];

    /* default to combined log format */
    if (fmt == NULL)
        fmt = "%h %l %u %t \"%r\" %s %b \"%{Referer}i\" \"%{User-agent}i\"";
    if ((logconf = h2o_logconf_compile(fmt, escape, errbuf)) == NULL) {
        h2o_error_printf("%s\n", errbuf);
        return NULL;
    }

    /* open log file */
    if ((fd = h2o_access_log_open_log(path)) == -1) {
        h2o_logconf_dispose(logconf);
        return NULL;
    }

    fh = h2o_mem_alloc_shared(NULL, sizeof(*fh), on_dispose_handle);
    fh->logconf = logconf;
    fh->fd = fd;
    return fh;
}


// Source: access_log.c
// Lines 133-158
