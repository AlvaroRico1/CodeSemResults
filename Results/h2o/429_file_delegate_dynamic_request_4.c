static int delegate_dynamic_request(h2o_req_t *req, h2o_iovec_t script_name, h2o_iovec_t path_info, const char *local_path,
                                    size_t local_path_len, h2o_mimemap_type_t *mime_type)
{
    h2o_filereq_t *filereq;
    h2o_handler_t *handler;

    assert(mime_type->data.dynamic.pathconf.handlers.size == 1);

    filereq = h2o_mem_alloc_pool(&req->pool, *filereq, 1);
    filereq->script_name = script_name;
    filereq->path_info = path_info;
    filereq->local_path = h2o_strdup(&req->pool, local_path, local_path_len);

    h2o_req_bind_conf(req, req->hostconf, &mime_type->data.dynamic.pathconf);
    req->filereq = filereq;

    handler = mime_type->data.dynamic.pathconf.handlers.entries[0];
    return handler->on_req(handler, req);
}


// Source: file.c
// Lines 553-571
