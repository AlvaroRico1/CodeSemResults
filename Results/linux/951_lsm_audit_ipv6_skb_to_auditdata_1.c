int ipv6_skb_to_auditdata(struct sk_buff *skb,
		struct common_audit_data *ad, u8 *proto)
{
	int offset, ret = 0;
	struct ipv6hdr *ip6;
	u8 nexthdr;
	__be16 frag_off;

	ip6 = ipv6_hdr(skb);
	if (ip6 == NULL)
		return -EINVAL;
	ad->u.net->v6info.saddr = ip6->saddr;
	ad->u.net->v6info.daddr = ip6->daddr;
	ret = 0;
	/* IPv6 can have several extension header before the Transport header
	 * skip them */
	offset = skb_network_offset(skb);
	offset += sizeof(*ip6);
	nexthdr = ip6->nexthdr;
	offset = ipv6_skip_exthdr(skb, offset, &nexthdr, &frag_off);
	if (offset < 0)
		return 0;
	if (proto)
		*proto = nexthdr;
	switch (nexthdr) {
	case IPPROTO_TCP: {
		struct tcphdr _tcph, *th;

		th = skb_header_pointer(skb, offset, sizeof(_tcph), &_tcph);
		if (th == NULL)
			break;

		ad->u.net->sport = th->source;
		ad->u.net->dport = th->dest;
		break;
	}
	case IPPROTO_UDP: {
		struct udphdr _udph, *uh;

		uh = skb_header_pointer(skb, offset, sizeof(_udph), &_udph);
		if (uh == NULL)
			break;

		ad->u.net->sport = uh->source;
		ad->u.net->dport = uh->dest;
		break;
	}
	case IPPROTO_DCCP: {
		struct dccp_hdr _dccph, *dh;

		dh = skb_header_pointer(skb, offset, sizeof(_dccph), &_dccph);
		if (dh == NULL)
			break;

		ad->u.net->sport = dh->dccph_sport;
		ad->u.net->dport = dh->dccph_dport;
		break;
	}
	case IPPROTO_SCTP: {
		struct sctphdr _sctph, *sh;

		sh = skb_header_pointer(skb, offset, sizeof(_sctph), &_sctph);
		if (sh == NULL)
			break;
		ad->u.net->sport = sh->source;
		ad->u.net->dport = sh->dest;
		break;
	}
	default:
		ret = -EINVAL;
	}
	return ret;
}


// Source: lsm_audit.c
// Lines 108-180
