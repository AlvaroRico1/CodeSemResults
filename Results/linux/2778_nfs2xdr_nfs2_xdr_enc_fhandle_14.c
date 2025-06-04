static void nfs2_xdr_enc_fhandle(struct rpc_rqst *req,
				 struct xdr_stream *xdr,
				 const void *data)
{
	const struct nfs_fh *fh = data;

	encode_fhandle(xdr, fh);
}


// Source: nfs2xdr.c
// Lines 557-564
