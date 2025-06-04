static void nfs4_xdr_enc_setclientid_confirm(struct rpc_rqst *req,
					     struct xdr_stream *xdr,
					     const void *data)
{
	const struct nfs4_setclientid_res *arg = data;
	struct compound_hdr hdr = {
		.nops	= 0,
	};

	encode_compound_hdr(xdr, req, &hdr);
	encode_setclientid_confirm(xdr, arg, &hdr);
	encode_nops(&hdr);
}


// Source: nfs4xdr.c
// Lines 2750-2762
