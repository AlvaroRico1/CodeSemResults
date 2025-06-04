static int nfs4_xdr_dec_fsinfo(struct rpc_rqst *req, struct xdr_stream *xdr,
			       void *data)
{
	struct nfs4_fsinfo_res *res = data;
	struct compound_hdr hdr;
	int status;

	status = decode_compound_hdr(xdr, &hdr);
	if (!status)
		status = decode_sequence(xdr, &res->seq_res, req);
	if (!status)
		status = decode_putfh(xdr);
	if (!status)
		status = decode_fsinfo(xdr, res->fsinfo);
	return status;
}


// Source: nfs4xdr.c
// Lines 6770-6785
