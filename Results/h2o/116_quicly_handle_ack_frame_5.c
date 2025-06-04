static int handle_ack_frame(quicly_conn_t *conn, struct st_quicly_handle_payload_state_t *state)
{
    quicly_ack_frame_t frame;
    quicly_sentmap_iter_t iter;
    struct {
        uint64_t pn;
        int64_t sent_at;
    } largest_newly_acked = {UINT64_MAX, INT64_MAX};
    size_t bytes_acked = 0;
    int includes_ack_eliciting = 0, ret;

    if ((ret = quicly_decode_ack_frame(&state->src, state->end, &frame, state->frame_type == QUICLY_FRAME_TYPE_ACK_ECN)) != 0)
        return ret;

    uint64_t pn_acked = frame.smallest_acknowledged;

    switch (state->epoch) {
    case QUICLY_EPOCH_0RTT:
        return QUICLY_TRANSPORT_ERROR_PROTOCOL_VIOLATION;
    case QUICLY_EPOCH_HANDSHAKE:
        conn->super.remote.address_validation.send_probe = 0;
        break;
    default:
        break;
    }

    init_acks_iter(conn, &iter);

    /* TODO log PNs being ACKed too late */

    size_t gap_index = frame.num_gaps;
    while (1) {
        assert(frame.ack_block_lengths[gap_index] != 0);
        /* Ack blocks are organized in the ACK frame and consequently in the ack_block_lengths array from the largest acked down.
         * Processing acks in packet number order requires processing the ack blocks in reverse order. */
        uint64_t pn_block_max = pn_acked + frame.ack_block_lengths[gap_index] - 1;
        QUICLY_PROBE(ACK_BLOCK_RECEIVED, conn, conn->stash.now, pn_acked, pn_block_max);
        while (quicly_sentmap_get(&iter)->packet_number < pn_acked)
            quicly_sentmap_skip(&iter);
        do {
            const quicly_sent_packet_t *sent = quicly_sentmap_get(&iter);
            uint64_t pn_sent = sent->packet_number;
            assert(pn_acked <= pn_sent);
            if (pn_acked < pn_sent) {
                /* set pn_acked to pn_sent; or past the end of the ack block, for use with the next ack block */
                if (pn_sent <= pn_block_max) {
                    pn_acked = pn_sent;
                } else {
                    pn_acked = pn_block_max + 1;
                    break;
                }
            }
            /* process newly acked packet */
            if (state->epoch != sent->ack_epoch)
                return QUICLY_TRANSPORT_ERROR_PROTOCOL_VIOLATION;
            int is_late_ack = 0;
            if (sent->ack_eliciting) {
                includes_ack_eliciting = 1;
                if (sent->cc_bytes_in_flight == 0) {
                    is_late_ack = 1;
                    ++conn->super.stats.num_packets.late_acked;
                }
            }
            ++conn->super.stats.num_packets.ack_received;
            largest_newly_acked.pn = pn_acked;
            largest_newly_acked.sent_at = sent->sent_at;
            QUICLY_PROBE(PACKET_ACKED, conn, conn->stash.now, pn_acked, is_late_ack);
            if (sent->cc_bytes_in_flight != 0) {
                bytes_acked += sent->cc_bytes_in_flight;
                conn->super.stats.num_bytes.ack_received += sent->cc_bytes_in_flight;
            }
            if ((ret = quicly_sentmap_update(&conn->egress.loss.sentmap, &iter, QUICLY_SENTMAP_EVENT_ACKED)) != 0)
                return ret;
            if (state->epoch == QUICLY_EPOCH_1RTT) {
                struct st_quicly_application_space_t *space = conn->application;
                if (space->cipher.egress.key_update_pn.last <= pn_acked) {
                    space->cipher.egress.key_update_pn.last = UINT64_MAX;
                    space->cipher.egress.key_update_pn.next = conn->egress.packet_number + conn->super.ctx->max_packets_per_key;
                    QUICLY_PROBE(CRYPTO_SEND_KEY_UPDATE_CONFIRMED, conn, conn->stash.now, space->cipher.egress.key_update_pn.next);
                }
            }


// Source: quicly.c
// Lines 4894-4974
