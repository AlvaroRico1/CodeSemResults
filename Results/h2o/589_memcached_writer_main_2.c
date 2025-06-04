static void *writer_main(void *_conn)
{
    struct st_h2o_memcached_conn_t *conn = _conn;
    yrmcds_error err;

    pthread_mutex_lock(&conn->ctx->mutex);

    while (!__sync_add_and_fetch(&conn->writer_exit_requested, 0)) {
        while (!h2o_linklist_is_empty(&conn->ctx->pending)) {
            h2o_memcached_req_t *req = H2O_STRUCT_FROM_MEMBER(h2o_memcached_req_t, pending, conn->ctx->pending.next);
            h2o_linklist_unlink(&req->pending);
            pthread_mutex_unlock(&conn->ctx->mutex);

            switch (req->type) {
            case REQ_TYPE_GET:
                pthread_mutex_lock(&conn->mutex);
                h2o_linklist_insert(&conn->inflight, &req->inflight);
                pthread_mutex_unlock(&conn->mutex);
                if ((err = yrmcds_get(&conn->yrmcds, req->key.base, req->key.len, 0, &req->data.get.serial)) != YRMCDS_OK)
                    goto Error;
                break;
            case REQ_TYPE_SET:
                err = yrmcds_set(&conn->yrmcds, req->key.base, req->key.len, req->data.set.value.base, req->data.set.value.len, 0,
                                 req->data.set.expiration, 0, !conn->yrmcds.text_mode, NULL);
                discard_req(req);
                if (err != YRMCDS_OK)
                    goto Error;
                break;
            case REQ_TYPE_DELETE:
                err = yrmcds_remove(&conn->yrmcds, req->key.base, req->key.len, !conn->yrmcds.text_mode, NULL);
                discard_req(req);
                if (err != YRMCDS_OK)
                    goto Error;
                break;
            default:
                h2o_error_printf("[lib/common/memcached.c] unknown type:%d\n", (int)req->type);
                err = YRMCDS_NOT_IMPLEMENTED;
                goto Error;
            }

            pthread_mutex_lock(&conn->ctx->mutex);
        }
        pthread_cond_wait(&conn->ctx->cond, &conn->ctx->mutex);
    }

    pthread_mutex_unlock(&conn->ctx->mutex);
    return NULL;

Error:
    h2o_error_printf("[lib/common/memcached.c] failed to send request; %s\n", yrmcds_strerror(err));
    /* doc says the call can be used to interrupt yrmcds_recv */
    yrmcds_shutdown(&conn->yrmcds);

    return NULL;
}


// Source: memcached.c
// Lines 167-221
