void quicly_amend_ptls_context(ptls_context_t *ptls)
{
    static ptls_update_traffic_key_t update_traffic_key = {update_traffic_key_cb};

    ptls->omit_end_of_early_data = 1;
    ptls->update_traffic_key = &update_traffic_key;

    /* if TLS 1.3 config permits use of early data, convert the value to 0xffffffff in accordance with QUIC-TLS */
    if (ptls->max_early_data_size != 0)
        ptls->max_early_data_size = UINT32_MAX;
}


// Source: quicly.c
// Lines 6228-6238
