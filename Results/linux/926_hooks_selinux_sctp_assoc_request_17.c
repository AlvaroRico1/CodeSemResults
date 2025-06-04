static int selinux_sctp_assoc_request(struct sctp_endpoint *ep,
				      struct sk_buff *skb)
{
	struct sk_security_struct *sksec = ep->base.sk->sk_security;
	struct common_audit_data ad;
	struct lsm_network_audit net = {0,};
	u8 peerlbl_active;
	u32 peer_sid = SECINITSID_UNLABELED;
	u32 conn_sid;
	int err = 0;

	if (!selinux_policycap_extsockclass())
		return 0;

	peerlbl_active = selinux_peerlbl_enabled();

	if (peerlbl_active) {
		/* This will return peer_sid = SECSID_NULL if there are
		 * no peer labels, see security_net_peersid_resolve().
		 */
		err = selinux_skb_peerlbl_sid(skb, ep->base.sk->sk_family,
					      &peer_sid);
		if (err)
			return err;

		if (peer_sid == SECSID_NULL)
			peer_sid = SECINITSID_UNLABELED;
	}

	if (sksec->sctp_assoc_state == SCTP_ASSOC_UNSET) {
		sksec->sctp_assoc_state = SCTP_ASSOC_SET;

		/* Here as first association on socket. As the peer SID
		 * was allowed by peer recv (and the netif/node checks),
		 * then it is approved by policy and used as the primary
		 * peer SID for getpeercon(3).
		 */
		sksec->peer_sid = peer_sid;
	} else if  (sksec->peer_sid != peer_sid) {
		/* Other association peer SIDs are checked to enforce
		 * consistency among the peer SIDs.
		 */
		ad.type = LSM_AUDIT_DATA_NET;
		ad.u.net = &net;
		ad.u.net->sk = ep->base.sk;
		err = avc_has_perm(&selinux_state,
				   sksec->peer_sid, peer_sid, sksec->sclass,
				   SCTP_SOCKET__ASSOCIATION, &ad);
		if (err)
			return err;
	}

	/* Compute the MLS component for the connection and store
	 * the information in ep. This will be used by SCTP TCP type
	 * sockets and peeled off connections as they cause a new
	 * socket to be generated. selinux_sctp_sk_clone() will then
	 * plug this into the new socket.
	 */
	err = selinux_conn_sid(sksec->sid, peer_sid, &conn_sid);
	if (err)
		return err;

	ep->secid = conn_sid;
	ep->peer_secid = peer_sid;

	/* Set any NetLabel labels including CIPSO/CALIPSO options. */
	return selinux_netlbl_sctp_assoc_request(ep, skb);
}


// Source: hooks.c
// Lines 5134-5201
