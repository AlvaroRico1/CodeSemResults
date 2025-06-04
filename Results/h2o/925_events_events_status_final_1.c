static h2o_iovec_t events_status_final(void *priv, h2o_globalconf_t *gconf, h2o_req_t *req)
{
    struct st_events_status_ctx_t *esc = priv;
    h2o_iovec_t ret;

#define H1_AGG_ERR(status_) esc->emitted_status_errors[H2O_STATUS_ERROR_##status_]
#define H2_AGG_ERR(err_) esc->h2_protocol_level_errors[-H2O_HTTP2_ERROR_##err_]
#define QUIC_FMT(_unused, label) " \"quic." label "\": %" PRIu64 ",\n"
#define QUIC_VAL(fld, _unused) , esc->quic_stats.quicly.fld
#define BUFSIZE (8 * 1024)
    ret.base = h2o_mem_alloc_pool(&req->pool, char, BUFSIZE);
    /* clang-format off */
    ret.len = snprintf(ret.base, BUFSIZE, ",\n"
                                          " \"status-errors.400\": %" PRIu64 ",\n"
                                          " \"status-errors.403\": %" PRIu64 ",\n"
                                          " \"status-errors.404\": %" PRIu64 ",\n"
                                          " \"status-errors.405\": %" PRIu64 ",\n"
                                          " \"status-errors.416\": %" PRIu64 ",\n"
                                          " \"status-errors.417\": %" PRIu64 ",\n"
                                          " \"status-errors.500\": %" PRIu64 ",\n"
                                          " \"status-errors.502\": %" PRIu64 ",\n"
                                          " \"status-errors.503\": %" PRIu64 ",\n"
                                          " \"http1-errors.request-timeout\": %" PRIu64 ",\n"
                                          " \"http1-errors.request-io-timeout\": %" PRIu64 ",\n"
                                          " \"http2-errors.protocol\": %" PRIu64 ",\n"
                                          " \"http2-errors.internal\": %" PRIu64 ",\n"
                                          " \"http2-errors.flow-control\": %" PRIu64 ",\n"
                                          " \"http2-errors.settings-timeout\": %" PRIu64 ",\n"
                                          " \"http2-errors.stream-closed\": %" PRIu64 ",\n"
                                          " \"http2-errors.frame-size\": %" PRIu64 ",\n"
                                          " \"http2-errors.refused-stream\": %" PRIu64 ",\n"
                                          " \"http2-errors.cancel\": %" PRIu64 ",\n"
                                          " \"http2-errors.compression\": %" PRIu64 ",\n"
                                          " \"http2-errors.connect\": %" PRIu64 ",\n"
                                          " \"http2-errors.enhance-your-calm\": %" PRIu64 ",\n"
                                          " \"http2-errors.inadequate-security\": %" PRIu64 ",\n"
                                          " \"http2.read-closed\": %" PRIu64 ",\n"
                                          " \"http2.write-closed\": %" PRIu64 ",\n"
                                          " \"http2.idle-timeout\": %" PRIu64 ",\n"
                                          " \"http2.streaming-requests\": %" PRIu64 ",\n"
                                          " \"http3.packet-forwarded\": %" PRIu64 ",\n"
                                          " \"http3.forwarded-packet-received\": %" PRIu64 ",\n"
                                          " \"quic.packet-received\": %" PRIu64 ",\n"
                                          " \"quic.packet-processed\": %" PRIu64
                                          ",\n" H2O_QUIC_AGGREGATED_STATS_APPLY(QUIC_FMT) " \"ssl.errors\": %" PRIu64 ",\n"
                                                                                          " \"memory.mmap_errors\": %zu\n",
                       H1_AGG_ERR(400), H1_AGG_ERR(403), H1_AGG_ERR(404), H1_AGG_ERR(405), H1_AGG_ERR(416), H1_AGG_ERR(417),
                       H1_AGG_ERR(500), H1_AGG_ERR(502), H1_AGG_ERR(503), esc->h1_request_timeout, esc->h1_request_io_timeout,
                       H2_AGG_ERR(PROTOCOL), H2_AGG_ERR(INTERNAL), H2_AGG_ERR(FLOW_CONTROL), H2_AGG_ERR(SETTINGS_TIMEOUT),
                       H2_AGG_ERR(STREAM_CLOSED), H2_AGG_ERR(FRAME_SIZE), H2_AGG_ERR(REFUSED_STREAM), H2_AGG_ERR(CANCEL),
                       H2_AGG_ERR(COMPRESSION), H2_AGG_ERR(CONNECT), H2_AGG_ERR(ENHANCE_YOUR_CALM), H2_AGG_ERR(INADEQUATE_SECURITY),
                       esc->h2_read_closed, esc->h2_write_closed, esc->h2_idle_timeout, esc->h2_streaming_requests,
                       esc->http3.packet_forwarded, esc->http3.forwarded_packet_received, esc->quic_stats.packet_received, esc->quic_stats.packet_processed
                       H2O_QUIC_AGGREGATED_STATS_APPLY(QUIC_VAL),
                       esc->ssl_errors, h2o_mmap_errors);
    /* clang-format on */
    assert(ret.len < BUFSIZE);
#undef H1_AGG_ERR
#undef H2_AGG_ERR
#undef QUIC_FMT
#undef QUIC_VAL
#undef BUFSIZE

    pthread_mutex_destroy(&esc->mutex);
    free(esc);
    return ret;
}


// Source: events.c
// Lines 86-152
