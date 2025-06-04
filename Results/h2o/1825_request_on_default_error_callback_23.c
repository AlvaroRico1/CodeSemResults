static void on_default_error_callback(void *data, h2o_iovec_t prefix, h2o_iovec_t msg)
{
    h2o_req_t *req = (void *)data;
    if (req->error_logs == NULL)
        h2o_buffer_init(&req->error_logs, &h2o_socket_buffer_prototype);
    h2o_buffer_append(&req->error_logs, prefix.base, prefix.len);
    h2o_buffer_append(&req->error_logs, msg.base, msg.len);

    if (req->pathconf->error_log.emit_request_errors) {
        h2o_write_error_log(prefix, msg);
    }
}


// Source: request.c
// Lines 242-253
