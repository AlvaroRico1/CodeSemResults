static __be32 decode_recall_args(struct svc_rqst *rqstp,
		struct xdr_stream *xdr, void *argp)
{
	struct cb_recallargs *args = argp;
	__be32 *p;
	__be32 status;

	status = decode_delegation_stateid(xdr, &args->stateid);
	if (unlikely(status != 0))
		return status;
	p = xdr_inline_decode(xdr, 4);
	if (unlikely(p == NULL))
		return htonl(NFS4ERR_RESOURCE);
	args->truncate = ntohl(*p);
	return decode_fh(xdr, &args->fh);
}


// Source: callback_xdr.c
// Lines 188-203
