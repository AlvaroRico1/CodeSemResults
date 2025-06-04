static void decr_active_cnt(h2o_http2_scheduler_node_t *node)
{
    h2o_http2_scheduler_openref_t *ref;

    /* do nothing if node is the root */
    if (node->_parent == NULL)
        return;

    ref = (h2o_http2_scheduler_openref_t *)node;
    if (--ref->_active_cnt != 0)
        return;
    /* just changed to inactive */
    queue_unset(&ref->_queue_node);
    /* delegate the change towards root */
    decr_active_cnt(ref->node._parent);
}


// Source: scheduler.c
// Lines 159-174
