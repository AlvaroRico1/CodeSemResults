static int nfs4_xdr_dec_open_confirm(struct rpc_rqst *rqstp,
				     struct xdr_stream *xdr,
				     void *data)
{
	struct nfs_open_confirmres *res = data;
	struct compound_hdr hdr;
	int status;

	status = decode_compound_hdr(xdr, &hdr);
	if (status)
		goto out;
	status = decode_putfh(xdr);
	if (status)
		goto out;
	status = decode_open_confirm(xdr, res);
out:
	return status;
}


// Source: nfs4xdr.c
// Lines 6472-6489
