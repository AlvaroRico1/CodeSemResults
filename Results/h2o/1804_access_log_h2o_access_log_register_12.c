h2o_logger_t *h2o_access_log_register(h2o_pathconf_t *pathconf, h2o_access_log_filehandle_t *fh)
{
    struct st_h2o_access_logger_t *self = (void *)h2o_create_logger(pathconf, sizeof(*self));

    self->super.dispose = dispose;
    self->super.log_access = log_access;
    self->fh = fh;
    h2o_mem_addref_shared(fh);

    return &self->super;
}


// Source: access_log.c
// Lines 167-177
