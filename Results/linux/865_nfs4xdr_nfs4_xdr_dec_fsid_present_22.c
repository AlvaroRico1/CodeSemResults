static int nfs4_xdr_dec_fsid_present(struct rpc_rqst *rqstp,
				     struct xdr_stream *xdr,
				     void *data)
{
	struct nfs4_fsid_present_res *res = data;
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
	status = decode_getfh(xdr, res->fh);
	if (status)
		goto out;
	if (res->renew)
		status = decode_renew(xdr);
out:
	return status;
}


// Source: nfs4xdr.c
// Lines 7008-7032
