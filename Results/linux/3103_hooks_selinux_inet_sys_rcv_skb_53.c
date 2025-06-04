static int selinux_inet_sys_rcv_skb(struct net *ns, int ifindex,
				    char *addrp, u16 family, u32 peer_sid,
				    struct common_audit_data *ad)
{
	int err;
	u32 if_sid;
	u32 node_sid;

	err = sel_netif_sid(ns, ifindex, &if_sid);
	if (err)
		return err;
	err = avc_has_perm(&selinux_state,
			   peer_sid, if_sid,
			   SECCLASS_NETIF, NETIF__INGRESS, ad);
	if (err)
		return err;

	err = sel_netnode_sid(addrp, family, &node_sid);
	if (err)
		return err;
	return avc_has_perm(&selinux_state,
			    peer_sid, node_sid,
			    SECCLASS_NODE, NODE__RECVFROM, ad);
}


// Source: hooks.c
// Lines 4875-4898
