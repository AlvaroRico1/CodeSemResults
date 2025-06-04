static void on_handler_dispose(h2o_handler_t *_self)
{
    h2o_file_handler_t *self = (void *)_self;
    size_t i;

    free(self->conf_path.base);
    free(self->real_path.base);
    h2o_mem_release_shared(self->mimemap);
    for (i = 0; self->index_files[i].base != NULL; ++i)
        free(self->index_files[i].base);
}


// Source: file.c
// Lines 874-884
