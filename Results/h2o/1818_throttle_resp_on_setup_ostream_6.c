static void on_setup_ostream(h2o_filter_t *self, h2o_req_t *req, h2o_ostream_t **slot)
{
    throttle_resp_t *throttle;
    size_t bytes_per_sec;

    /* only handle 200 OK with content */
    if (req->res.status != 200)
        goto Next;
    if (h2o_memis(req->input.method.base, req->input.method.len, H2O_STRLIT("HEAD")))
        goto Next;

    { /* obtain the rate from X-Traffic header field and delete the header field, or skip */
        ssize_t xt_index;
        if ((xt_index = h2o_find_header(&req->res.headers, H2O_TOKEN_X_TRAFFIC, -1)) == -1)
            goto Next;
        char *buf = req->res.headers.entries[xt_index].value.base;
        if (H2O_UNLIKELY((bytes_per_sec = h2o_strtosizefwd(&buf, req->res.headers.entries[xt_index].value.len)) == SIZE_MAX))
            goto Next;
        h2o_delete_header(&req->res.headers, xt_index);
    }

    /* instantiate the ostream filter */
    throttle = (void *)h2o_add_ostream(req, H2O_ALIGNOF(*throttle), sizeof(*throttle), slot);
    throttle->super.do_send = on_send;
    throttle->super.stop = on_stop;
    h2o_timer_init(&throttle->timeout_entry, on_timer);
    throttle->window.at = h2o_now(req->conn->ctx->loop);
    throttle->window.bytes_left = 0;
    throttle->req = req;
    throttle->bytes_per_sec = bytes_per_sec;
    memset(&throttle->state.bufs, 0, sizeof(throttle->state.bufs));
    throttle->state.stream_state = H2O_SEND_STATE_IN_PROGRESS;

    { /* reduce `preferred_chunk_size` so that we'd be sending one chunk every 100ms */
        size_t chunk_size = bytes_per_sec / 10;
        if (chunk_size < 4096)
            chunk_size = 4096;
        if (req->preferred_chunk_size > chunk_size)
            req->preferred_chunk_size = chunk_size;
    }

    slot = &throttle->super.next;

Next:
    h2o_setup_next_ostream(req, slot);
}


// Source: throttle_resp.c
// Lines 107-152
