static void incr_active_cnt(h2o_http2_scheduler_node_t *node)
{
    h2o_http2_scheduler_openref_t *ref;

    /* do nothing if node is the root */
    if (node->_parent == NULL)
        return;

    ref = (h2o_http2_scheduler_openref_t *)node;
    if (++ref->_active_cnt != 1)
        return;
    /* just changed to active */
    queue_set(get_queue(ref->node._parent), &ref->_queue_node, ref->weight);
    /* delegate the change towards root */
    incr_active_cnt(ref->node._parent);
}


// Source: scheduler.c
// Lines 142-157
