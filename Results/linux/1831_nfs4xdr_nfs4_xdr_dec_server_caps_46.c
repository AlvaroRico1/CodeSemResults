static int nfs4_xdr_dec_server_caps(struct rpc_rqst *req,
				    struct xdr_stream *xdr,
				    void *data)
{
	struct nfs4_server_caps_res *res = data;
	struct compound_hdr hdr;
	int status;

	status = decode_compound_hdr(xdr, &hdr);
	if (status)
		goto out;
	status = decode_sequence(xdr, &res->seq_res, req);
	if (status)
		goto out;
	status = decode_putfh(xdr);
	if (status)
		goto out;
	status = decode_server_caps(xdr, res);
out:
	return status;
}


// Source: nfs4xdr.c
// Lines 6830-6850
