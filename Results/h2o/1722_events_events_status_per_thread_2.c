static void events_status_per_thread(void *priv, h2o_context_t *ctx)
{
    size_t i;
    struct st_events_status_ctx_t *esc = priv;

    pthread_mutex_lock(&esc->mutex);

    for (i = 0; i < H2O_STATUS_ERROR_MAX; i++) {
        esc->emitted_status_errors[i] += ctx->emitted_error_status[i];
    }
    esc->ssl_errors += ctx->ssl.errors;
    for (i = 0; i < H2O_HTTP2_ERROR_MAX; i++) {
        esc->h2_protocol_level_errors[i] += ctx->http2.events.protocol_level_errors[i];
    }
    esc->h2_read_closed += ctx->http2.events.read_closed;
    esc->h2_write_closed += ctx->http2.events.write_closed;
    esc->h2_idle_timeout += ctx->http2.events.idle_timeouts;
    esc->h2_streaming_requests += ctx->http2.events.streaming_requests;
    esc->h1_request_timeout += ctx->http1.events.request_timeouts;
    esc->h1_request_io_timeout += ctx->http1.events.request_io_timeouts;
    esc->http3.packet_forwarded += ctx->http3.events.packet_forwarded;
    esc->http3.forwarded_packet_received += ctx->http3.events.forwarded_packet_received;
    esc->quic_stats.packet_received += ctx->quic_stats.packet_received;
    esc->quic_stats.packet_processed += ctx->quic_stats.packet_processed;
#define ACC(fld, _unused) esc->quic_stats.quicly.fld += ctx->quic_stats.quicly.fld;
    H2O_QUIC_AGGREGATED_STATS_APPLY(ACC);
#undef ACC

    pthread_mutex_unlock(&esc->mutex);
}


// Source: events.c
// Lines 44-73
