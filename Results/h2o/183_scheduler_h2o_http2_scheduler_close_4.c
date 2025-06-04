void h2o_http2_scheduler_close(h2o_http2_scheduler_openref_t *ref)
{
    assert(h2o_http2_scheduler_is_open(ref));

    /* move dependents to parent */
    if (!h2o_linklist_is_empty(&ref->node._all_refs)) {
        /* proportionally distribute the weight to the children (draft-16 5.3.4) */
        uint32_t total_weight = 0, factor;
        h2o_linklist_t *link;
        for (link = ref->node._all_refs.next; link != &ref->node._all_refs; link = link->next) {
            h2o_http2_scheduler_openref_t *child_ref = H2O_STRUCT_FROM_MEMBER(h2o_http2_scheduler_openref_t, _all_link, link);
            total_weight += child_ref->weight;
        }
        assert(total_weight != 0);
        factor = ((uint32_t)ref->weight * 65536 + total_weight / 2) / total_weight;
        do {
            h2o_http2_scheduler_openref_t *child_ref =
                H2O_STRUCT_FROM_MEMBER(h2o_http2_scheduler_openref_t, _all_link, ref->node._all_refs.next);
            uint16_t weight = (child_ref->weight * factor / 32768 + 1) / 2;
            if (weight < 1)
                weight = 1;
            else if (weight > 256)
                weight = 256;
            h2o_http2_scheduler_rebind(child_ref, ref->node._parent, weight, 0);
        } while (!h2o_linklist_is_empty(&ref->node._all_refs));
    }


// Source: scheduler.c
// Lines 207-232
