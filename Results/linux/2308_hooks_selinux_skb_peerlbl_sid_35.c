static int selinux_skb_peerlbl_sid(struct sk_buff *skb, u16 family, u32 *sid)
{
	int err;
	u32 xfrm_sid;
	u32 nlbl_sid;
	u32 nlbl_type;

	err = selinux_xfrm_skb_sid(skb, &xfrm_sid);
	if (unlikely(err))
		return -EACCES;
	err = selinux_netlbl_skbuff_getsid(skb, family, &nlbl_type, &nlbl_sid);
	if (unlikely(err))
		return -EACCES;

	err = security_net_peersid_resolve(&selinux_state, nlbl_sid,
					   nlbl_type, xfrm_sid, sid);
	if (unlikely(err)) {
		pr_warn(
		       "SELinux: failure in selinux_skb_peerlbl_sid(),"
		       " unable to determine packet's peer label\n");
		return -EACCES;
	}

	return 0;
}


// Source: hooks.c
// Lines 4357-4381
