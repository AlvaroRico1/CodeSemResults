static void nfs3_xdr_enc_read3args(struct rpc_rqst *req,
				   struct xdr_stream *xdr,
				   const void *data)
{
	const struct nfs_pgio_args *args = data;
	unsigned int replen = args->replen ? args->replen : NFS3_readres_sz;

	encode_read3args(xdr, args);
	rpc_prepare_reply_pages(req, args->pages, args->pgbase,
				args->count, replen);
	req->rq_rcv_buf.flags |= XDRBUF_READ;
}


// Source: nfs3xdr.c
// Lines 941-952
