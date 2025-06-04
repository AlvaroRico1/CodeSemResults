static h2o_iovec_t durations_status_final(void *priv, h2o_globalconf_t *gconf, h2o_req_t *req)
{
    struct st_duration_agg_stats_t *agg_stats = priv;
    h2o_iovec_t ret;

#define BUFSIZE 16384
#define DURATION_FMT(x)                                                                                                            \
    " \"" x "-0\": %lu,\n"                                                                                                         \
    " \"" x "-25\": %lu,\n"                                                                                                        \
    " \"" x "-50\": %lu,\n"                                                                                                        \
    " \"" x "-75\": %lu,\n"                                                                                                        \
    " \"" x "-99\": %lu\n"
#define DURATION_VALS(x)                                                                                                           \
    gkc_query(agg_stats->stats.x, 0), gkc_query(agg_stats->stats.x, 0.25), gkc_query(agg_stats->stats.x, 0.5),                     \
        gkc_query(agg_stats->stats.x, 0.75), gkc_query(agg_stats->stats.x, 0.99)

    ret.base = h2o_mem_alloc_pool(&req->pool, char, BUFSIZE);
    ret.len = snprintf(
        ret.base, BUFSIZE,
        ",\n" DURATION_FMT("connect-time") "," DURATION_FMT("header-time") "," DURATION_FMT("body-time") "," DURATION_FMT(
            "request-total-time") "," DURATION_FMT("process-time") "," DURATION_FMT("response-time") "," DURATION_FMT("duration"),
        DURATION_VALS(connect_time), DURATION_VALS(header_time), DURATION_VALS(body_time), DURATION_VALS(request_total_time),
        DURATION_VALS(process_time), DURATION_VALS(response_time), DURATION_VALS(total_time));
#undef DURATION_FMT
#undef DURATION_VALS
    char *delim = "";
    ret.len += sprintf(ret.base + ret.len, ",\n\"evloop-latency-nanosec\": [");
    size_t i;
    for (i = 0; i < agg_stats->stats.evloop_latency_nanosec.size; i++) {
        size_t len = snprintf(NULL, 0, "%s%" PRIu64, delim, agg_stats->stats.evloop_latency_nanosec.entries[i]);
        /* require that there's enough space for the closing "]\0" */
        if (ret.len + len + 1 >= BUFSIZE)
            break;
        ret.len += snprintf(ret.base + ret.len, BUFSIZE - ret.len, "%s%" PRIu64, delim,
                            agg_stats->stats.evloop_latency_nanosec.entries[i]);
        delim = ",";
    }
    ret.len += snprintf(ret.base + ret.len, BUFSIZE - ret.len, "]");
#undef BUFSIZE
    duration_stats_free(&agg_stats->stats);
    pthread_mutex_destroy(&agg_stats->mutex);

    free(agg_stats);
    return ret;
}


// Source: durations.c
// Lines 119-163
