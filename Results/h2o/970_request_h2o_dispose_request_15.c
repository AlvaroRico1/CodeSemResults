void h2o_dispose_request(h2o_req_t *req)
{
    close_generator_and_filters(req);

    h2o_timer_unlink(&req->_timeout_entry);

    if (req->pathconf != NULL) {
        h2o_logger_t **logger = req->loggers, **end = logger + req->num_loggers;
        for (; logger != end; ++logger) {
            (*logger)->log_access((*logger), req);
        }
    }


// Source: request.c
// Lines 324-335
