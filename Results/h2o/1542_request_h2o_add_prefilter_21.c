h2o_req_prefilter_t *h2o_add_prefilter(h2o_req_t *req, size_t alignment, size_t sz)
{
    h2o_req_prefilter_t *prefilter = h2o_mem_alloc_pool_aligned(&req->pool, alignment, sz);
    prefilter->next = req->prefilters;
    req->prefilters = prefilter;
    return prefilter;
}


// Source: request.c
// Lines 580-586
