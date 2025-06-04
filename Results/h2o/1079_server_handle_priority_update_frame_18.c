static int handle_priority_update_frame(struct st_h2o_http3_server_conn_t *conn, const h2o_http3_priority_update_frame_t *frame)
{
    if (frame->element_is_push)
        return H2O_HTTP3_ERROR_GENERAL_PROTOCOL;

    /* obtain the stream being referred to (creating one if necessary), or return if the stream has been closed already */
    quicly_stream_t *qs;
    if (quicly_get_or_open_stream(conn->h3.super.quic, frame->element, &qs) != 0)
        return H2O_HTTP3_ERROR_ID;
    if (qs == NULL)
        return 0;

    /* apply the changes */
    struct st_h2o_http3_server_stream_t *stream = qs->data;
    assert(stream != NULL);
    stream->received_priority_update = 1;
    if (h2o_linklist_is_linked(&stream->scheduler.link)) {
        req_scheduler_deactivate(&conn->scheduler.reqs, &stream->scheduler);
        stream->scheduler.priority = frame->priority; /* TODO apply only the delta? */
        req_scheduler_activate(&conn->scheduler.reqs, &stream->scheduler, req_scheduler_compare_stream_id);
    } else {
        stream->scheduler.priority = frame->priority; /* TODO apply only the delta? */
    }

    return 0;
}


// Source: server.c
// Lines 1436-1461
