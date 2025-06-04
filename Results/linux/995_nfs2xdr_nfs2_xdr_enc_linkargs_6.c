static void nfs2_xdr_enc_linkargs(struct rpc_rqst *req,
				  struct xdr_stream *xdr,
				  const void *data)
{
	const struct nfs_linkargs *args = data;

	encode_fhandle(xdr, args->fromfh);
	encode_diropargs(xdr, args->tofh, args->toname, args->tolen);
}


// Source: nfs2xdr.c
// Lines 736-744
