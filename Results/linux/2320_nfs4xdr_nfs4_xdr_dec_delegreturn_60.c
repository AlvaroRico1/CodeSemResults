static int nfs4_xdr_dec_delegreturn(struct rpc_rqst *rqstp,
				    struct xdr_stream *xdr,
				    void *data)
{
	struct nfs4_delegreturnres *res = data;
	struct compound_hdr hdr;
	int status;

	status = decode_compound_hdr(xdr, &hdr);
	if (status)
		goto out;
	status = decode_sequence(xdr, &res->seq_res, rqstp);
	if (status)
		goto out;
	status = decode_putfh(xdr);
	if (status != 0)
		goto out;
	if (res->lr_res) {
		status = decode_layoutreturn(xdr, res->lr_res);
		res->lr_ret = status;
		if (status)
			goto out;
	}
	if (res->fattr) {
		status = decode_getfattr(xdr, res->fattr, res->server);
		if (status != 0)
			goto out;
	}
	status = decode_delegreturn(xdr);
out:
	return status;
}


// Source: nfs4xdr.c
// Lines 6903-6934
