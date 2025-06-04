static int handle_input_data_payload(struct st_h2o_http3client_req_t *req, const uint8_t **src, const uint8_t *src_end, int err,
                                     const char **err_desc)
{
    /* save data, update states */
    if (req->bytes_left_in_data_frame != 0) {
        size_t payload_bytes = req->bytes_left_in_data_frame;
        if (src_end - *src < payload_bytes)
            payload_bytes = src_end - *src;
        h2o_buffer_append(&req->recvbuf.body, *src, payload_bytes);
        *src += payload_bytes;
        req->bytes_left_in_data_frame -= payload_bytes;
    }


// Source: http3client.c
// Lines 408-419
