static void nfs3_xdr_enc_remove3args(struct rpc_rqst *req,
				     struct xdr_stream *xdr,
				     const void *data)
{
	const struct nfs_removeargs *args = data;

	encode_diropargs3(xdr, args->fh, args->name.name, args->name.len);
}


// Source: nfs3xdr.c
// Lines 1169-1176
