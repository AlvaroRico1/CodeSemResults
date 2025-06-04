static h2o_memcached_req_t *pop_inflight(struct st_h2o_memcached_conn_t *conn, uint32_t serial)
{
    h2o_memcached_req_t *req;

    pthread_mutex_lock(&conn->mutex);

    if (conn->yrmcds.text_mode) {
        /* in text mode, responses are returned in order (and we may receive responses for commands other than GET) */
        if (!h2o_linklist_is_empty(&conn->inflight)) {
            req = H2O_STRUCT_FROM_MEMBER(h2o_memcached_req_t, inflight, conn->inflight.next);
            assert(req->type == REQ_TYPE_GET);
            if (req->data.get.serial == serial)
                goto Found;
        }
    } else {
        /* in binary mode, responses are received out-of-order (and we would only recieve responses for GET) */
        h2o_linklist_t *node;
        for (node = conn->inflight.next; node != &conn->inflight; node = node->next) {
            req = H2O_STRUCT_FROM_MEMBER(h2o_memcached_req_t, inflight, node);
            assert(req->type == REQ_TYPE_GET);
            if (req->data.get.serial == serial)
                goto Found;
        }
    }


// Source: memcached.c
// Lines 132-155
