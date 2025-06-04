static void on_getaddr(h2o_hostinfo_getaddr_req_t *getaddr_req, const char *errstr, struct addrinfo *res, void *_req)
{
    h2o_socketpool_connect_request_t *req = _req;

    assert(getaddr_req == req->getaddr_req);
    req->getaddr_req = NULL;

    if (errstr != NULL) {
        __sync_sub_and_fetch(&req->pool->targets.entries[req->selected_target]->_shared.leased_count, 1);
        if (req->remaining_try_count > 0) {
            try_connect(req);
            return;
        }
        __sync_sub_and_fetch(&req->pool->_shared.count, 1);
        call_connect_cb(req, errstr);
        return;
    }

    struct addrinfo *selected = h2o_hostinfo_select_one(res);
    start_connect(req, selected->ai_addr, selected->ai_addrlen);
}


// Source: socketpool.c
// Lines 422-442
