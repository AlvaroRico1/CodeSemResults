void h2o_buffer_consume_all(h2o_buffer_t **inbuf, int record_capacity)
{
    if ((*inbuf)->size != 0) {
        if (record_capacity) {
            h2o_buffer_t *newp = h2o_mem_alloc_recycle(&buffer_recycle_bins.zero_sized, sizeof(*newp));
            buffer_init(newp, 0, NULL, (*inbuf)->capacity, (*inbuf)->_prototype, -1);
            h2o_buffer__do_free(*inbuf);
            *inbuf = newp;
        } else {
            h2o_buffer_t *prototype_buf = &(*inbuf)->_prototype->_initial_buf;
            h2o_buffer__do_free(*inbuf);
            *inbuf = prototype_buf;
        }


// Source: memory.c
// Lines 485-497
