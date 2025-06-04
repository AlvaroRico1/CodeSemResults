static void nfs3_xdr_enc_write3args(struct rpc_rqst *req,
				    struct xdr_stream *xdr,
				    const void *data)
{
	const struct nfs_pgio_args *args = data;

	encode_write3args(xdr, args);
	xdr->buf->flags |= XDRBUF_WRITE;
}


// Source: nfs3xdr.c
// Lines 986-994
