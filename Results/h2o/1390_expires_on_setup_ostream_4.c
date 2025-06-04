static void on_setup_ostream(h2o_filter_t *_self, h2o_req_t *req, h2o_ostream_t **slot)
{
    struct st_expires_t *self = (void *)_self;

    switch (req->res.status) {
    case 200:
    case 201:
    case 204:
    case 206:
    case 301:
    case 302:
    case 303:
    case 304:
    case 307:
        switch (self->mode) {
        case H2O_EXPIRES_MODE_ABSOLUTE:
            h2o_set_header(&req->pool, &req->res.headers, H2O_TOKEN_EXPIRES, self->value.base, self->value.len, 0);
            break;
        case H2O_EXPIRES_MODE_MAX_AGE:
            h2o_set_header_token(&req->pool, &req->res.headers, H2O_TOKEN_CACHE_CONTROL, self->value.base, self->value.len);
            break;
        default:
            assert(0);
            break;
        }
        break;
    default:
        break;
    }

    h2o_setup_next_ostream(req, slot);
}


// Source: expires.c
// Lines 34-65
