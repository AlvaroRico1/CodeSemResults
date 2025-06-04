static void dispose(h2o_logger_t *_self)
{
    struct st_h2o_access_logger_t *self = (void *)_self;

    h2o_mem_release_shared(self->fh);
}


// Source: access_log.c
// Lines 160-165
