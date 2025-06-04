static h2o_iovec_t requests_status_final(void *priv, h2o_globalconf_t *gconf, h2o_req_t *req)
{
    h2o_iovec_t ret = {NULL};
    struct st_requests_status_ctx_t *rsc = priv;

    if (rsc->logconf != NULL) {
        ret = h2o_concat(&req->pool, h2o_iovec_init(H2O_STRLIT(",\n \"requests\": [")), rsc->req_data,
                         h2o_iovec_init(H2O_STRLIT("\n ]")));
        h2o_logconf_dispose(rsc->logconf);
    }
    free(rsc->req_data.base);
    pthread_mutex_destroy(&rsc->mutex);

    free(rsc);
    return ret;
}


// Source: requests.c
// Lines 145-160
