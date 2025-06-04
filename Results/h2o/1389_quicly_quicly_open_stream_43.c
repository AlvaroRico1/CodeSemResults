int quicly_open_stream(quicly_conn_t *conn, quicly_stream_t **_stream, int uni)
{
    quicly_stream_t *stream;
    struct st_quicly_conn_streamgroup_state_t *group;
    uint64_t *max_stream_count;
    uint32_t max_stream_data_local;
    uint64_t max_stream_data_remote;
    int ret;

    /* determine the states */
    if (uni) {
        group = &conn->super.local.uni;
        max_stream_count = &conn->egress.max_streams.uni.count;
        max_stream_data_local = 0;
        max_stream_data_remote = conn->super.remote.transport_params.max_stream_data.uni;
    } else {
        group = &conn->super.local.bidi;
        max_stream_count = &conn->egress.max_streams.bidi.count;
        max_stream_data_local = (uint32_t)conn->super.ctx->transport_params.max_stream_data.bidi_local;
        max_stream_data_remote = conn->super.remote.transport_params.max_stream_data.bidi_remote;
    }

    /* open */
    if ((stream = open_stream(conn, group->next_stream_id, max_stream_data_local, max_stream_data_remote)) == NULL)
        return PTLS_ERROR_NO_MEMORY;
    ++group->num_streams;
    group->next_stream_id += 4;

    /* adjust blocked */
    if (stream->stream_id / 4 >= *max_stream_count) {
        stream->streams_blocked = 1;
        quicly_linklist_insert((uni ? &conn->egress.pending_streams.blocked.uni : &conn->egress.pending_streams.blocked.bidi)->prev,
                               &stream->_send_aux.pending_link.control);
    }

    /* application-layer initialization */
    QUICLY_PROBE(STREAM_ON_OPEN, conn, conn->stash.now, stream);
    if ((ret = conn->super.ctx->stream_open->cb(conn->super.ctx->stream_open, stream)) != 0)
        return ret;

    *_stream = stream;
    return 0;
}


// Source: quicly.c
// Lines 6067-6109
