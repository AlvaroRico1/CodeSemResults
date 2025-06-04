static void nfs4_xdr_enc_symlink(struct rpc_rqst *req, struct xdr_stream *xdr,
				 const void *data)
{
	const struct nfs4_create_arg *args = data;

	nfs4_xdr_enc_create(req, xdr, args);
}


// Source: nfs4xdr.c
// Lines 2277-2283
