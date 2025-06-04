static int nfs3_xdr_dec_fsinfo3res(struct rpc_rqst *req,
				   struct xdr_stream *xdr,
				   void *data)
{
	struct nfs_fsinfo *result = data;
	enum nfs_stat status;
	int error;

	error = decode_nfsstat3(xdr, &status);
	if (unlikely(error))
		goto out;
	error = decode_post_op_attr(xdr, result->fattr, rpc_rqst_userns(req));
	if (unlikely(error))
		goto out;
	if (status != NFS3_OK)
		goto out_status;
	error = decode_fsinfo3resok(xdr, result);
out:
	return error;
out_status:
	return nfs3_stat_to_errno(status);
}


// Source: nfs3xdr.c
// Lines 2233-2254
