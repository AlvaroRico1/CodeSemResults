static void nfs3_xdr_enc_readlink3args(struct rpc_rqst *req,
				       struct xdr_stream *xdr,
				       const void *data)
{
	const struct nfs3_readlinkargs *args = data;

	encode_nfs_fh3(xdr, args->fh);
	rpc_prepare_reply_pages(req, args->pages, args->pgbase,
				args->pglen, NFS3_readlinkres_sz);
}


// Source: nfs3xdr.c
// Lines 909-918
