static void nfs2_xdr_enc_writeargs(struct rpc_rqst *req,
				   struct xdr_stream *xdr,
				   const void *data)
{
	const struct nfs_pgio_args *args = data;

	encode_writeargs(xdr, args);
	xdr->buf->flags |= XDRBUF_WRITE;
}


// Source: nfs2xdr.c
// Lines 671-679
