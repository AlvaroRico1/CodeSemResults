static void nfs3_xdr_enc_readdir3args(struct rpc_rqst *req,
				      struct xdr_stream *xdr,
				      const void *data)
{
	const struct nfs3_readdirargs *args = data;

	encode_readdir3args(xdr, args);
	rpc_prepare_reply_pages(req, args->pages, 0,
				args->count, NFS3_readdirres_sz);
}


// Source: nfs3xdr.c
// Lines 1239-1248
