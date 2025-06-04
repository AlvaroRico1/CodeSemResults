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
    if (path_info.base != NULL)
        append_pair(&req->pool, vecs, H2O_STRLIT("PATH_INFO"), path_info.base, path_info.len);
    /* DOCUMENT_ROOT and PATH_TRANSLATED */
    if (config->document_root.base != NULL) {
        append_pair(&req->pool, vecs, H2O_STRLIT("DOCUMENT_ROOT"), config->document_root.base, config->document_root.len);
        if (path_info.base != NULL) {
            append_pair(&req->pool, vecs, H2O_STRLIT("PATH_TRANSLATED"), NULL, config->document_root.len + path_info.len);
            char *dst_end = vecs->entries[vecs->size - 1].base + vecs->entries[vecs->size - 1].len;
            memcpy(dst_end - path_info.len, path_info.base, path_info.len);
            memcpy(dst_end - path_info.len - config->document_root.len, config->document_root.base, config->document_root.len);
        }
    }
    /* QUERY_STRING (and adjust PATH_INFO) */
    if (req->query_at != SIZE_MAX) {
        append_pair(&req->pool, vecs, H2O_STRLIT("QUERY_STRING"), req->path.base + req->query_at + 1,
                    req->path.len - (req->query_at + 1));
    } else {
        append_pair(&req->pool, vecs, H2O_STRLIT("QUERY_STRING"), NULL, 0);
    }
    /* REMOTE_ADDR & REMOTE_PORT */
    append_address_info(req, vecs, H2O_STRLIT("REMOTE_ADDR"), H2O_STRLIT("REMOTE_PORT"), req->conn->callbacks->get_peername);
    { /* environment variables (REMOTE_USER, etc.) */
        size_t i;
        for (i = 0; i != req->env.size; i += 2) {
            h2o_iovec_t *name = req->env.entries + i, *value = name + 1;
            append_pair(&req->pool, vecs, name->base, name->len, value->base, value->len);
        }


// Source: fastcgi.c
// Lines 216-262
