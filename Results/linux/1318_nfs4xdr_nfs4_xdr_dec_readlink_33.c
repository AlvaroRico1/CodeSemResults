static int nfs4_xdr_dec_readlink(struct rpc_rqst *rqstp,
				 struct xdr_stream *xdr,
				 void *data)
{
	struct nfs4_readlink_res *res = data;
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
	status = decode_readlink(xdr, rqstp);
out:
	return status;
}


// Source: nfs4xdr.c
// Lines 6638-6658
