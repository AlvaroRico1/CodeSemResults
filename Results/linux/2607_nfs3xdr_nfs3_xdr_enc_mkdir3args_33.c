static void nfs3_xdr_enc_mkdir3args(struct rpc_rqst *req,
				    struct xdr_stream *xdr,
				    const void *data)
{
	const struct nfs3_mkdirargs *args = data;

	encode_diropargs3(xdr, args->fh, args->name, args->len);
	encode_sattr3(xdr, args->sattr, rpc_rqst_userns(req));
}


// Source: nfs3xdr.c
// Lines 1054-1062
