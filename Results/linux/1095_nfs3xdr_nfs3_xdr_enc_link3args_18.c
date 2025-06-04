static void nfs3_xdr_enc_link3args(struct rpc_rqst *req,
				   struct xdr_stream *xdr,
				   const void *data)
{
	const struct nfs3_linkargs *args = data;

	encode_nfs_fh3(xdr, args->fromfh);
	encode_diropargs3(xdr, args->tofh, args->toname, args->tolen);
}


// Source: nfs3xdr.c
// Lines 1206-1214
