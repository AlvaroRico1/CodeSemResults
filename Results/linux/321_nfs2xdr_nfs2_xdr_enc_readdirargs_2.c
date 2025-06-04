static void nfs2_xdr_enc_readdirargs(struct rpc_rqst *req,
				     struct xdr_stream *xdr,
				     const void *data)
{
	const struct nfs_readdirargs *args = data;

	encode_readdirargs(xdr, args);
	rpc_prepare_reply_pages(req, args->pages, 0,
				args->count, NFS_readdirres_sz);
}


// Source: nfs2xdr.c
// Lines 787-796
