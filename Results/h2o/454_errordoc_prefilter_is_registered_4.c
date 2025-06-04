static int prefilter_is_registered(h2o_req_t *req)
{
    h2o_req_prefilter_t *prefilter;
    for (prefilter = req->prefilters; prefilter != NULL; prefilter = prefilter->next)
        if (prefilter->on_setup_ostream == on_prefilter_setup_stream)
            return 1;
    return 0;
}


// Source: errordoc.c
// Lines 75-82
