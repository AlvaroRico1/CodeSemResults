static void nfs3_xdr_enc_symlink3args(struct rpc_rqst *req,
				      struct xdr_stream *xdr,
				      const void *data)
{
	const struct nfs3_symlinkargs *args = data;

	encode_diropargs3(xdr, args->fromfh, args->fromname, args->fromlen);
	encode_symlinkdata3(xdr, args, rpc_rqst_userns(req));
	xdr->buf->flags |= XDRBUF_WRITE;
}


// Source: nfs3xdr.c
// Lines 1087-1096
