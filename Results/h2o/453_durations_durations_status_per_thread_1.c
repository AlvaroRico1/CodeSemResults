static void durations_status_per_thread(void *priv, h2o_context_t *ctx)
{
    struct st_duration_agg_stats_t *agg_stats = priv;
    if (durations_logger) {
        struct st_duration_stats_t *ctx_stats = h2o_context_get_logger_context(ctx, durations_logger);
        pthread_mutex_lock(&agg_stats->mutex);
#define ADD_DURATION(x)                                                                                                            \
    do {                                                                                                                           \
        struct gkc_summary *tmp;                                                                                                   \
        tmp = gkc_combine(agg_stats->stats.x, ctx_stats->x);                                                                       \
        gkc_summary_free(agg_stats->stats.x);                                                                                      \
        agg_stats->stats.x = tmp;                                                                                                  \
    } while (0)
        ADD_DURATION(connect_time);
        ADD_DURATION(header_time);
        ADD_DURATION(body_time);
        ADD_DURATION(request_total_time);
        ADD_DURATION(process_time);
        ADD_DURATION(response_time);
        ADD_DURATION(total_time);
#undef ADD_DURATION

#if !H2O_USE_LIBUV
        h2o_vector_reserve(NULL, &agg_stats->stats.evloop_latency_nanosec, agg_stats->stats.evloop_latency_nanosec.size + 1);
        agg_stats->stats.evloop_latency_nanosec.entries[agg_stats->stats.evloop_latency_nanosec.size] =
            h2o_evloop_get_execution_time_nanosec(ctx->loop);
        agg_stats->stats.evloop_latency_nanosec.size++;
#endif
        pthread_mutex_unlock(&agg_stats->mutex);
    }
}


// Source: durations.c
// Lines 51-81
