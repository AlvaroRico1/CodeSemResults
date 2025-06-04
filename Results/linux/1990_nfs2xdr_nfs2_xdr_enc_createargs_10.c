static void nfs2_xdr_enc_createargs(struct rpc_rqst *req,
				    struct xdr_stream *xdr,
				    const void *data)
{
	const struct nfs_createargs *args = data;

	encode_diropargs(xdr, args->fh, args->name, args->len);
	encode_sattr(xdr, args->sattr, rpc_rqst_userns(req));
}


// Source: nfs2xdr.c
// Lines 689-697
