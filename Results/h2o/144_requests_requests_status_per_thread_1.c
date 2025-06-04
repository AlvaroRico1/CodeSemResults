static void requests_status_per_thread(void *priv, h2o_context_t *ctx)
{
    struct st_requests_status_ctx_t *rsc = priv;
    struct st_collect_req_status_cbdata_t cbdata = {rsc->logconf};

    /* we encountered an error at init() time, return early */
    if (rsc->logconf == NULL)
        return;

    h2o_buffer_init(&cbdata.buffer, &h2o_socket_buffer_prototype);
    if (ctx->globalconf->http1.callbacks.foreach_request(ctx, collect_req_status, &cbdata) != 0 ||
        ctx->globalconf->http2.callbacks.foreach_request(ctx, collect_req_status, &cbdata) != 0 ||
        ctx->globalconf->http3.callbacks.foreach_request(ctx, collect_req_status, &cbdata) != 0) {
        h2o_buffer_dispose(&cbdata.buffer);
        return;
    }

    /* concat JSON elements */
    if (cbdata.buffer->size != 0) {
        pthread_mutex_lock(&rsc->mutex);
        if (rsc->req_data.len == 0)
            h2o_buffer_consume(&cbdata.buffer, 1); /* skip preceeding comma */
        rsc->req_data.base = h2o_mem_realloc(rsc->req_data.base, rsc->req_data.len + cbdata.buffer->size);
        memcpy(rsc->req_data.base + rsc->req_data.len, cbdata.buffer->bytes, cbdata.buffer->size);
        rsc->req_data.len += cbdata.buffer->size;
        pthread_mutex_unlock(&rsc->mutex);
    }

    h2o_buffer_dispose(&cbdata.buffer);
}


// Source: requests.c
// Lines 62-91
