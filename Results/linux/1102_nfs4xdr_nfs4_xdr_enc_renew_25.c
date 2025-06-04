static void nfs4_xdr_enc_renew(struct rpc_rqst *req, struct xdr_stream *xdr,
			       const void *data)

{
	const struct nfs_client *clp = data;
	struct compound_hdr hdr = {
		.nops	= 0,
	};

	encode_compound_hdr(xdr, req, &hdr);
	encode_renew(xdr, clp->cl_clientid, &hdr);
	encode_nops(&hdr);
}


// Source: nfs4xdr.c
// Lines 2716-2728
