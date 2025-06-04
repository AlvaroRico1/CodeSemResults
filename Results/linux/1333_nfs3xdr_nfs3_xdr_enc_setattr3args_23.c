static void nfs3_xdr_enc_setattr3args(struct rpc_rqst *req,
				      struct xdr_stream *xdr,
				      const void *data)
{
	const struct nfs3_sattrargs *args = data;
	encode_nfs_fh3(xdr, args->fh);
	encode_sattr3(xdr, args->sattr, rpc_rqst_userns(req));
	encode_sattrguard3(xdr, args);
}


// Source: nfs3xdr.c
// Lines 852-860
