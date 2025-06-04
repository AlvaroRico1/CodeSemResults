static void nfs2_xdr_enc_symlinkargs(struct rpc_rqst *req,
				     struct xdr_stream *xdr,
				     const void *data)
{
	const struct nfs_symlinkargs *args = data;

	encode_diropargs(xdr, args->fromfh, args->fromname, args->fromlen);
	encode_path(xdr, args->pages, args->pathlen);
	encode_sattr(xdr, args->sattr, rpc_rqst_userns(req));
}


// Source: nfs2xdr.c
// Lines 755-764
