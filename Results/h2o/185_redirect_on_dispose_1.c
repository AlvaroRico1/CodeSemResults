static void on_dispose(h2o_handler_t *_self)
{
    h2o_redirect_handler_t *self = (void *)_self;
    size_t i;
    for (i = 0; i != self->prefix_list.size; ++i) {
        free(self->prefix_list.entries[i].base);
    }
    free(self->prefix_list.entries);
}


// Source: redirect.c
// Lines 36-44
