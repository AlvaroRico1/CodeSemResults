static void errorclose(struct st_fcgi_generator_t *generator)
{
    if (generator->sent_headers) {
        send_eos_and_close(generator, 0);
    } else {
        h2o_req_t *req = generator->req;
        close_generator(generator);
        h2o_send_error_503(req, "Internal Server Error", "Internal Server Error", 0);
    }


// Source: fastcgi.c
// Lines 476-484
