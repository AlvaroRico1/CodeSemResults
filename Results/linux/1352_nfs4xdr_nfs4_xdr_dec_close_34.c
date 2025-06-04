static int nfs4_xdr_dec_close(struct rpc_rqst *rqstp, struct xdr_stream *xdr,
			      void *data)
{
	struct nfs_closeres *res = data;
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
	if (res->lr_res) {
		status = decode_layoutreturn(xdr, res->lr_res);
		res->lr_ret = status;
		if (status)
			goto out;
	}
	if (res->fattr != NULL) {
		status = decode_getfattr(xdr, res->fattr, res->server);
		if (status != 0)
			goto out;
	}
	status = decode_close(xdr, res);
out:
	return status;
}


// Source: nfs4xdr.c
// Lines 6403-6433
