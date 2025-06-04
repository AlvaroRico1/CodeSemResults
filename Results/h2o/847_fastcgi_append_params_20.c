static void append_params(h2o_req_t *req, iovec_vector_t *vecs, h2o_fastcgi_config_vars_t *config)
{
    h2o_iovec_t path_info = {NULL};

    /* CONTENT_LENGTH */
    if (req->entity.base != NULL) {
        char buf[32];
        int l = sprintf(buf, "%zu", req->entity.len);
        append_pair(&req->pool, vecs, H2O_STRLIT("CONTENT_LENGTH"), buf, (size_t)l);
    }
    /* SCRIPT_FILENAME, SCRIPT_NAME, PATH_INFO */
    if (req->filereq != NULL) {
        h2o_filereq_t *filereq = req->filereq;
        append_pair(&req->pool, vecs, H2O_STRLIT("SCRIPT_FILENAME"), filereq->local_path.base, filereq->local_path.len);
        append_pair(&req->pool, vecs, H2O_STRLIT("SCRIPT_NAME"), filereq->script_name.base, filereq->script_name.len);
        path_info = filereq->path_info;
    } else {
        append_pair(&req->pool, vecs, H2O_STRLIT("SCRIPT_NAME"), NULL, 0);
        path_info = req->path_normalized;
    }


// Source: fastcgi.c
// Lines 216-235
