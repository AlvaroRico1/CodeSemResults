static void nfs2_xdr_enc_removeargs(struct rpc_rqst *req,
				    struct xdr_stream *xdr,
				    const void *data)
{
	const struct nfs_removeargs *args = data;

	encode_diropargs(xdr, args->fh, args->name.name, args->name.len);
}


// Source: nfs2xdr.c
// Lines 699-706
