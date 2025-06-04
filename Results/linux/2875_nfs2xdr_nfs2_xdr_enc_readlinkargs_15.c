static void nfs2_xdr_enc_readlinkargs(struct rpc_rqst *req,
				      struct xdr_stream *xdr,
				      const void *data)
{
	const struct nfs_readlinkargs *args = data;

	encode_fhandle(xdr, args->fh);
	rpc_prepare_reply_pages(req, args->pages, args->pgbase,
				args->pglen, NFS_readlinkres_sz);
}


// Source: nfs2xdr.c
// Lines 593-602
