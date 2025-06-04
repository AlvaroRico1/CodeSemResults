void h2o_replay_request(h2o_req_t *req)
{
    close_generator_and_filters(req);
    reset_response(req);

    if (req->handler != NULL) {
        h2o_handler_t **handler = req->pathconf->handlers.entries, **end = handler + req->pathconf->handlers.size;
        for (;; ++handler) {
            assert(handler != end);
            if (*handler == req->handler)
                break;
        }
        call_handlers(req, handler);
    } else {
        process_resolved_request(req, req->conn->hosts);
    }


// Source: request.c
// Lines 476-491
