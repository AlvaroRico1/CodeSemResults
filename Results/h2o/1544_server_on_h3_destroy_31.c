static void on_h3_destroy(h2o_quic_conn_t *h3_)
{
    h2o_http3_conn_t *h3 = (h2o_http3_conn_t *)h3_;
    struct st_h2o_http3_server_conn_t *conn = H2O_STRUCT_FROM_MEMBER(struct st_h2o_http3_server_conn_t, h3, h3);
    quicly_stats_t stats;

    H2O_PROBE_CONN0(H3S_DESTROY, &conn->super);

    if (quicly_get_stats(h3_->quic, &stats) == 0) {
#define ACC(fld, _unused) conn->super.ctx->quic_stats.quicly.fld += stats.fld;
        H2O_QUIC_AGGREGATED_STATS_APPLY(ACC);
#undef ACC
    }

    /* unlink and dispose */
    h2o_linklist_unlink(&conn->_conns);
    if (h2o_timer_is_linked(&conn->timeout))
        h2o_timer_unlink(&conn->timeout);
    h2o_http3_dispose_conn(&conn->h3);
    kh_destroy(stream, conn->datagram_flows);

    /* check consistency post-disposal */
    assert(conn->num_streams.recv_headers == 0);
    assert(conn->num_streams.req_pending == 0);
    assert(conn->num_streams.send_headers == 0);
    assert(conn->num_streams.send_body == 0);
    assert(conn->num_streams.close_wait == 0);
    assert(conn->num_streams_req_streaming == 0);
    assert(conn->num_streams_tunnelling == 0);
    assert(h2o_linklist_is_empty(&conn->delayed_streams.recv_body_blocked));
    assert(h2o_linklist_is_empty(&conn->delayed_streams.req_streaming));
    assert(h2o_linklist_is_empty(&conn->delayed_streams.pending));
    assert(conn->scheduler.reqs.active.smallest_urgency >= H2O_ABSPRIO_NUM_URGENCY_LEVELS);
    assert(h2o_linklist_is_empty(&conn->scheduler.reqs.conn_blocked));

    /* free memory */
    free(conn);
}


// Source: server.c
// Lines 1753-1790
