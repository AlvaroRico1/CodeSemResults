static void nfs3_xdr_enc_mknod3args(struct rpc_rqst *req,
				    struct xdr_stream *xdr,
				    const void *data)
{
	const struct nfs3_mknodargs *args = data;

	encode_diropargs3(xdr, args->fh, args->name, args->len);
	encode_mknoddata3(xdr, args, rpc_rqst_userns(req));
}


// Source: nfs3xdr.c
// Lines 1152-1160
