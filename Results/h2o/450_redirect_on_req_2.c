static int on_req(h2o_handler_t *_self, h2o_req_t *req)
{
    h2o_redirect_handler_t *self = (void *)_self;

    h2o_iovec_t delimiter =
        req->authority_wildcard_match.base == NULL ? h2o_iovec_init(H2O_STRLIT("*")) : req->authority_wildcard_match;
    h2o_iovec_t prefix = h2o_join_list(&req->pool, self->prefix_list.entries, self->prefix_list.size, delimiter);
    h2o_iovec_t dest = h2o_build_destination(req, prefix.base, prefix.len, 1);

    /* redirect */
    if (self->internal) {
        redirect_internally(self, req, dest);
    } else {
        h2o_send_redirect(req, self->status, "Redirected", dest.base, dest.len);
    }

    return 0;
}


// Source: redirect.c
// Lines 71-88
