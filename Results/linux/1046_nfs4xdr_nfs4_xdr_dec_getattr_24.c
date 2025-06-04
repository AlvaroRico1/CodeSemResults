static int nfs4_xdr_dec_getattr(struct rpc_rqst *rqstp, struct xdr_stream *xdr,
				void *data)
{
	struct nfs4_getattr_res *res = data;
	struct compound_hdr hdr;
	int status;

	status = decode_compound_hdr(xdr, &hdr);
	if (status)
		goto out;
	status = decode_sequence(xdr, &res->seq_res, rqstp);
	if (status)
		goto out;
	status = decode_putfh(xdr);
	if (status)
		goto out;
	status = decode_getfattr_label(xdr, res->fattr, res->label, res->server);
out:
	return status;
}


// Source: nfs4xdr.c
// Lines 6306-6325
