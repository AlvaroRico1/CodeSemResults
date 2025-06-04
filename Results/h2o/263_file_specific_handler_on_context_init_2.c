static void specific_handler_on_context_init(h2o_handler_t *_self, h2o_context_t *ctx)
{
    struct st_h2o_specific_file_handler_t *self = (void *)_self;

    if (self->mime_type->type == H2O_MIMEMAP_TYPE_DYNAMIC)
        h2o_context_init_pathconf_context(ctx, &self->mime_type->data.dynamic.pathconf);
}


// Source: file.c
// Lines 931-937
