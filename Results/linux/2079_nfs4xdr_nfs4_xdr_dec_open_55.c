static int nfs4_xdr_dec_open(struct rpc_rqst *rqstp, struct xdr_stream *xdr,
			     void *data)
{
	struct nfs_openres *res = data;
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
	status = decode_open(xdr, res);
	if (status)
		goto out;
	status = decode_getfh(xdr, &res->fh);
	if (status)
		goto out;
	if (res->access_request)
		decode_access(xdr, &res->access_supported, &res->access_result);
	decode_getfattr_label(xdr, res->f_attr, res->f_label, res->server);
	if (res->lg_res)
		decode_layoutget(xdr, rqstp, res->lg_res);
out:
	return status;
}


// Source: nfs4xdr.c
// Lines 6438-6467
