void h2o_http2_scheduler_rebind(h2o_http2_scheduler_openref_t *ref, h2o_http2_scheduler_node_t *new_parent, uint16_t weight,
                                int exclusive)
{
    assert(h2o_http2_scheduler_is_open(ref));
    assert(&ref->node != new_parent);
    assert(1 <= weight);
    assert(weight <= 257);

    /* do nothing if there'd be no change at all */
    if (ref->node._parent == new_parent && ref->weight == weight && !exclusive)
        return;

    ref->weight = weight;

    /* if new_parent is dependent to ref, make new_parent a sibling of ref before applying the final transition (see draft-16
       5.3.3) */
    if (!h2o_linklist_is_empty(&ref->node._all_refs)) {
        h2o_http2_scheduler_node_t *t;
        for (t = new_parent; t->_parent != NULL; t = t->_parent) {
            if (t->_parent == &ref->node) {
                /* quoting the spec: "The moved dependency retains its weight." */
                h2o_http2_scheduler_openref_t *new_parent_ref = (void *)new_parent;
                do_rebind(new_parent_ref, ref->node._parent, 0);
                break;
            }
        }
    }


// Source: scheduler.c
// Lines 307-333
