static void nfs2_xdr_enc_diropargs(struct rpc_rqst *req,
				   struct xdr_stream *xdr,
				   const void *data)
{
	const struct nfs_diropargs *args = data;

	encode_diropargs(xdr, args->fh, args->name, args->len);
}


// Source: nfs2xdr.c
// Lines 584-591
