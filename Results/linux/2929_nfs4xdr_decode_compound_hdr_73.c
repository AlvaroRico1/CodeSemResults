static int decode_compound_hdr(struct xdr_stream *xdr, struct compound_hdr *hdr)
{
	__be32 *p;

	p = xdr_inline_decode(xdr, 8);
	if (unlikely(!p))
		return -EIO;
	hdr->status = be32_to_cpup(p++);
	hdr->taglen = be32_to_cpup(p);

	p = xdr_inline_decode(xdr, hdr->taglen + 4);
	if (unlikely(!p))
		return -EIO;
	hdr->tag = (char *)p;
	p += XDR_QUADLEN(hdr->taglen);
	hdr->nops = be32_to_cpup(p);
	if (unlikely(hdr->nops < 1))
		return nfs4_stat_to_errno(hdr->status);
	return 0;
}


// Source: nfs4xdr.c
// Lines 3156-3175
