static void specific_handler_on_dispose(h2o_handler_t *_self)
{
    struct st_h2o_specific_file_handler_t *self = (void *)_self;

    free(self->real_path.base);
    h2o_mem_release_shared(self->mime_type);
}


// Source: file.c
// Lines 947-953
