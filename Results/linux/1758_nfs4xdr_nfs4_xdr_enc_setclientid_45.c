static void nfs4_xdr_enc_setclientid(struct rpc_rqst *req,
				     struct xdr_stream *xdr,
				     const void *data)
{
	const struct nfs4_setclientid *sc = data;
	struct compound_hdr hdr = {
		.nops	= 0,
	};

	encode_compound_hdr(xdr, req, &hdr);
	encode_setclientid(xdr, sc, &hdr);
	encode_nops(&hdr);
}


// Source: nfs4xdr.c
// Lines 2733-2745
