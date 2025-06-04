static void nfs2_xdr_enc_readargs(struct rpc_rqst *req,
				  struct xdr_stream *xdr,
				  const void *data)
{
	const struct nfs_pgio_args *args = data;

	encode_readargs(xdr, args);
	rpc_prepare_reply_pages(req, args->pages, args->pgbase,
				args->count, NFS_readres_sz);
	req->rq_rcv_buf.flags |= XDRBUF_READ;
}


// Source: nfs2xdr.c
// Lines 629-639
