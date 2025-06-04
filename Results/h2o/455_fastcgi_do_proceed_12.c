static void do_proceed(h2o_generator_t *_generator, h2o_req_t *req)
{
    struct st_fcgi_generator_t *generator = (void *)_generator;

    h2o_doublebuffer_consume(&generator->resp.sending);
    do_send(generator);
}


// Source: fastcgi.c
// Lines 762-768
