static void nfs3_xdr_enc_commit3args(struct rpc_rqst *req,
				     struct xdr_stream *xdr,
				     const void *data)
{
	const struct nfs_commitargs *args = data;

	encode_commit3args(xdr, args);
}


// Source: nfs3xdr.c
// Lines 1313-1320
