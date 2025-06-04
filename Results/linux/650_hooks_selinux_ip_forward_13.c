static unsigned int selinux_ip_forward(struct sk_buff *skb,
				       const struct net_device *indev,
				       u16 family)
{
	int err;
	char *addrp;
	u32 peer_sid;
	struct common_audit_data ad;
	struct lsm_network_audit net = {0,};
	u8 secmark_active;
	u8 netlbl_active;
	u8 peerlbl_active;

	if (!selinux_policycap_netpeer())
		return NF_ACCEPT;

	secmark_active = selinux_secmark_enabled();
	netlbl_active = netlbl_enabled();
	peerlbl_active = selinux_peerlbl_enabled();
	if (!secmark_active && !peerlbl_active)
		return NF_ACCEPT;

	if (selinux_skb_peerlbl_sid(skb, family, &peer_sid) != 0)
		return NF_DROP;

	ad.type = LSM_AUDIT_DATA_NET;
	ad.u.net = &net;
	ad.u.net->netif = indev->ifindex;
	ad.u.net->family = family;
	if (selinux_parse_skb(skb, &ad, &addrp, 1, NULL) != 0)
		return NF_DROP;

	if (peerlbl_active) {
		err = selinux_inet_sys_rcv_skb(dev_net(indev), indev->ifindex,
					       addrp, family, peer_sid, &ad);
		if (err) {
			selinux_netlbl_err(skb, family, err, 1);
			return NF_DROP;
		}
	}

	if (secmark_active)
		if (avc_has_perm(&selinux_state,
				 peer_sid, skb->secmark,
				 SECCLASS_PACKET, PACKET__FORWARD_IN, &ad))
			return NF_DROP;

	if (netlbl_active)
		/* we do this in the FORWARD path and not the POST_ROUTING
		 * path because we want to make sure we apply the necessary
		 * labeling before IPsec is applied so we can leverage AH
		 * protection */
		if (selinux_netlbl_skbuff_setsid(skb, family, peer_sid) != 0)
			return NF_DROP;

	return NF_ACCEPT;
}


// Source: hooks.c
// Lines 5504-5560
