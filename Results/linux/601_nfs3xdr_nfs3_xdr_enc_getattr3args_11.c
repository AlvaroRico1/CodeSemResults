static void nfs3_xdr_enc_getattr3args(struct rpc_rqst *req,
				      struct xdr_stream *xdr,
				      const void *data)
{
	const struct nfs_fh *fh = data;

	encode_nfs_fh3(xdr, fh);
}


// Source: nfs3xdr.c
// Lines 812-819
