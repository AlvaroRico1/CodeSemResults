static void nfs3_xdr_enc_access3args(struct rpc_rqst *req,
				     struct xdr_stream *xdr,
				     const void *data)
{
	const struct nfs3_accessargs *args = data;

	encode_access3args(xdr, args);
}


// Source: nfs3xdr.c
// Lines 893-900
