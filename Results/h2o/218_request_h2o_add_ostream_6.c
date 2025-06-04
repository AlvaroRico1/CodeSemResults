h2o_ostream_t *h2o_add_ostream(h2o_req_t *req, size_t alignment, size_t sz, h2o_ostream_t **slot)
{
    h2o_ostream_t *ostr = h2o_mem_alloc_pool_aligned(&req->pool, alignment, sz);
    ostr->next = *slot;
    ostr->do_send = NULL;
    ostr->stop = NULL;
    ostr->send_informational = NULL;

    *slot = ostr;

    return ostr;
}


// Source: request.c
// Lines 588-599
