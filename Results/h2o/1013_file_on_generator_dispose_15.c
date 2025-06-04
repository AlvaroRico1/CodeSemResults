static void on_generator_dispose(void *_self)
{
    struct st_h2o_sendfile_generator_t *self = _self;
    close_file(self);
}


// Source: file.c
// Lines 121-125
