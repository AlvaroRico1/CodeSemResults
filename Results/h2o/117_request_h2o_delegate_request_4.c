void h2o_delegate_request(h2o_req_t *req)
{
    h2o_handler_t **handler = req->pathconf->handlers.entries, **end = handler + req->pathconf->handlers.size;
    for (;; ++handler) {
        assert(handler != end);
        if (*handler == req->handler)
            break;
    }
    ++handler;
    call_handlers(req, handler);
}


// Source: request.c
// Lines 377-387
