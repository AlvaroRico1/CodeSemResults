static void nfs3_xdr_enc_rename3args(struct rpc_rqst *req,
				     struct xdr_stream *xdr,
				     const void *data)
{
	const struct nfs_renameargs *args = data;
	const struct qstr *old = args->old_name;
	const struct qstr *new = args->new_name;

	encode_diropargs3(xdr, args->old_dir, old->name, old->len);
	encode_diropargs3(xdr, args->new_dir, new->name, new->len);
}


// Source: nfs3xdr.c
// Lines 1186-1196
