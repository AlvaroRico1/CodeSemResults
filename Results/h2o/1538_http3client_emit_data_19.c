static size_t emit_data(struct st_h2o_http3client_req_t *req, h2o_iovec_t payload)
{
    size_t nbytes;

    { /* emit header */
        uint8_t buf[9], *p = buf;
        *p++ = H2O_HTTP3_FRAME_TYPE_DATA;
        p = quicly_encodev(p, payload.len);
        nbytes = p - buf;
        h2o_buffer_append(&req->sendbuf, buf, nbytes);
    }


// Source: http3client.c
// Lines 123-133
