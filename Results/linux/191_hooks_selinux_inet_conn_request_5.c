static int selinux_inet_conn_request(struct sock *sk, struct sk_buff *skb,
				     struct request_sock *req)
{
	struct sk_security_struct *sksec = sk->sk_security;
	int err;
	u16 family = req->rsk_ops->family;
	u32 connsid;
	u32 peersid;

	err = selinux_skb_peerlbl_sid(skb, family, &peersid);
	if (err)
		return err;
	err = selinux_conn_sid(sksec->sid, peersid, &connsid);
	if (err)
		return err;
	req->secid = connsid;
	req->peer_secid = peersid;

	return selinux_netlbl_inet_conn_request(req, family);
}


// Source: hooks.c
// Lines 5303-5322
