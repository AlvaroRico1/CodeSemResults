static void nfs3_xdr_enc_lookup3args(struct rpc_rqst *req,
				     struct xdr_stream *xdr,
				     const void *data)
{
	const struct nfs3_diropargs *args = data;

	encode_diropargs3(xdr, args->fh, args->name, args->len);
}


// Source: nfs3xdr.c
// Lines 869-876
