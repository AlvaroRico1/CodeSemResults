static void on_context_dispose(h2o_handler_t *_self, h2o_context_t *ctx)
{
    h2o_file_handler_t *self = (void *)_self;

    h2o_mimemap_on_context_dispose(self->mimemap, ctx);
}


// Source: file.c
// Lines 867-872
