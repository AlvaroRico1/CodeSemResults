static int specific_handler_on_req(h2o_handler_t *_self, h2o_req_t *req)
{
    struct st_h2o_specific_file_handler_t *self = (void *)_self;
    struct st_h2o_sendfile_generator_t *generator;
    int is_dir;

    /* open file (or send error or return -1) */
    if ((generator = create_generator(req, self->real_path.base, self->real_path.len, &is_dir, self->flags)) == NULL) {
        if (is_dir) {
            h2o_send_error_403(req, "Access Forbidden", "access forbidden", 0);
        } else if (errno == ENOENT) {
            return -1;
        } else if (errno == ENFILE || errno == EMFILE) {
            h2o_send_error_503(req, "Service Unavailable", "please try again later", 0);
        } else {
            h2o_send_error_403(req, "Access Forbidden", "access forbidden", 0);
        }
        return 0;
    }

    return serve_with_generator(generator, req, req->path_normalized, self->real_path.base, self->real_path.len, self->mime_type);
}


// Source: file.c
// Lines 955-976
