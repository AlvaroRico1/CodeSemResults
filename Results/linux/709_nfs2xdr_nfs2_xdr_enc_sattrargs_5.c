static void nfs2_xdr_enc_sattrargs(struct rpc_rqst *req,
				   struct xdr_stream *xdr,
				   const void *data)
{
	const struct nfs_sattrargs *args = data;

	encode_fhandle(xdr, args->fh);
	encode_sattr(xdr, args->sattr, rpc_rqst_userns(req));
}


// Source: nfs2xdr.c
// Lines 574-582
