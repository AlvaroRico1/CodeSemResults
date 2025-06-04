static h2o_iovec_t log_quic_stats(h2o_req_t *req)
{
#define APPLY_NUM_FRAMES(f, dir)                                                                                                   \
    f(padding, dir) f(ping, dir) f(ack, dir) f(reset_stream, dir) f(stop_sending, dir) f(crypto, dir) f(new_token, dir)            \
        f(stream, dir) f(max_data, dir) f(max_stream_data, dir) f(max_streams_bidi, dir) f(max_streams_uni, dir)                   \
            f(data_blocked, dir) f(stream_data_blocked, dir) f(streams_blocked, dir) f(new_connection_id, dir)                     \
                f(retire_connection_id, dir) f(path_challenge, dir) f(path_response, dir) f(transport_close, dir)                  \
                    f(application_close, dir) f(handshake_done, dir) f(ack_frequency, dir)
#define FORMAT_OF_NUM_FRAMES(n, dir) "," H2O_TO_STR(n) "-" H2O_TO_STR(dir) "=%" PRIu64
#define VALUE_OF_NUM_FRAMES(n, dir) , stats.num_frames_##dir.n

    struct st_h2o_http3_server_conn_t *conn = (struct st_h2o_http3_server_conn_t *)req->conn;
    quicly_stats_t stats;

    if (quicly_get_stats(conn->h3.super.quic, &stats) != 0)
        return h2o_iovec_init(H2O_STRLIT("-"));

    char *buf;
    size_t len, bufsize = 1400;
Redo:
    buf = h2o_mem_alloc_pool(&req->pool, char, bufsize);
    len = snprintf(
        buf, bufsize,
        "packets-received=%" PRIu64 ",packets-decryption-failed=%" PRIu64 ",packets-sent=%" PRIu64 ",packets-lost=%" PRIu64
        ",packets-lost-time-threshold=%" PRIu64 ",packets-ack-received=%" PRIu64 ",late-acked=%" PRIu64 ",bytes-received=%" PRIu64
        ",bytes-sent=%" PRIu64 ",bytes-lost=%" PRIu64 ",bytes-ack-received=%" PRIu64 ",bytes-stream-data-sent=%" PRIu64
        ",bytes-stream-data-resent=%" PRIu64 ",rtt-minimum=%" PRIu32 ",rtt-smoothed=%" PRIu32 ",rtt-variance=%" PRIu32
        ",rtt-latest=%" PRIu32 ",cwnd=%" PRIu32 ",ssthresh=%" PRIu32 ",cwnd-initial=%" PRIu32 ",cwnd-exiting-slow-start=%" PRIu32
        ",cwnd-minimum=%" PRIu32 ",cwnd-maximum=%" PRIu32 ",num-loss-episodes=%" PRIu32 ",num-ptos=%" PRIu64
        ",delivery-rate-latest=%" PRIu64 ",delivery-rate-smoothed=%" PRIu64
        ",delivery-rate-stdev=%" PRIu64 APPLY_NUM_FRAMES(FORMAT_OF_NUM_FRAMES, received)
            APPLY_NUM_FRAMES(FORMAT_OF_NUM_FRAMES, sent),
        stats.num_packets.received, stats.num_packets.decryption_failed, stats.num_packets.sent, stats.num_packets.lost,
        stats.num_packets.lost_time_threshold, stats.num_packets.ack_received, stats.num_packets.late_acked,
        stats.num_bytes.received, stats.num_bytes.sent, stats.num_bytes.lost, stats.num_bytes.ack_received,
        stats.num_bytes.stream_data_sent, stats.num_bytes.stream_data_resent, stats.rtt.minimum, stats.rtt.smoothed,
        stats.rtt.variance, stats.rtt.latest, stats.cc.cwnd, stats.cc.ssthresh, stats.cc.cwnd_initial,
        stats.cc.cwnd_exiting_slow_start, stats.cc.cwnd_minimum, stats.cc.cwnd_maximum, stats.cc.num_loss_episodes, stats.num_ptos,
        stats.delivery_rate.latest, stats.delivery_rate.smoothed,
        stats.delivery_rate.stdev APPLY_NUM_FRAMES(VALUE_OF_NUM_FRAMES, received) APPLY_NUM_FRAMES(VALUE_OF_NUM_FRAMES, sent));
    if (len + 1 > bufsize) {
        bufsize = len + 1;
        goto Redo;
    }

    return h2o_iovec_init(buf, len);

#undef APPLY_NUM_FRAMES
#undef FORMAT_OF_NUM_FRAMES
#undef VALUE_OF_NUM_FRAMES
}


// Source: server.c
// Lines 602-652
