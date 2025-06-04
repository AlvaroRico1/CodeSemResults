static void nfs2_xdr_enc_renameargs(struct rpc_rqst *req,
				    struct xdr_stream *xdr,
				    const void *data)
{
	const struct nfs_renameargs *args = data;
	const struct qstr *old = args->old_name;
	const struct qstr *new = args->new_name;

	encode_diropargs(xdr, args->old_dir, old->name, old->len);
	encode_diropargs(xdr, args->new_dir, new->name, new->len);
}


// Source: nfs2xdr.c
// Lines 716-726
