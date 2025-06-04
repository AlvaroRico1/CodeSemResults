static int64_t get_rtt(h2o_conn_t *_conn)
{
    struct st_h2o_http2_conn_t *conn = (void *)_conn;
    if (!h2o_timeval_is_null(&conn->timestamps.settings_sent_at) && !h2o_timeval_is_null(&conn->timestamps.settings_acked_at)) {
        return h2o_timeval_subtract(&conn->timestamps.settings_sent_at, &conn->timestamps.settings_acked_at);
    } else {
        return -1;
    }
}


// Source: connection.c
// Lines 1574-1582
