static void on_dispose_handle(void *_fh)
{
    h2o_access_log_filehandle_t *fh = _fh;

    h2o_logconf_dispose(fh->logconf);
    close(fh->fd);
}


// Source: access_log.c
// Lines 63-69
