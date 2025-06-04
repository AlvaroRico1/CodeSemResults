static void record_socket_error(struct st_connect_generator_t *self, const char *err)
{
    const char *error_type;
    const char *details = NULL;
    if (err == h2o_socket_error_conn_refused)
        error_type = "connection_refused";
    else if (err == h2o_socket_error_conn_timed_out)
        error_type = "connection_timeout";
    else if (err == h2o_socket_error_network_unreachable || err == h2o_socket_error_host_unreachable)
        error_type = "destination_ip_unroutable";
    else {
        error_type = "proxy_internal_error";
        details = err;
    }
    record_error(self, error_type, details, NULL);
}


// Source: connect.c
// Lines 156-171
