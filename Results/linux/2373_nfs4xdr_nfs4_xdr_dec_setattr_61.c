static int nfs4_xdr_dec_setattr(struct rpc_rqst *rqstp,
				struct xdr_stream *xdr,
				void *data)
{
	struct nfs_setattrres *res = data;
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
	status = decode_setattr(xdr);
	if (status)
		goto out;
	decode_getfattr_label(xdr, res->fattr, res->label, res->server);
out:
	return status;
}


// Source: nfs4xdr.c
// Lines 6526-6549
