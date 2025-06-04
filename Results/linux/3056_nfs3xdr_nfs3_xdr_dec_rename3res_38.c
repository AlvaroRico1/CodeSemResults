static int nfs3_xdr_dec_rename3res(struct rpc_rqst *req,
				   struct xdr_stream *xdr,
				   void *data)
{
	struct user_namespace *userns = rpc_rqst_userns(req);
	struct nfs_renameres *result = data;
	enum nfs_stat status;
	int error;

	error = decode_nfsstat3(xdr, &status);
	if (unlikely(error))
		goto out;
	error = decode_wcc_data(xdr, result->old_fattr, userns);
	if (unlikely(error))
		goto out;
	error = decode_wcc_data(xdr, result->new_fattr, userns);
	if (unlikely(error))
		goto out;
	if (status != NFS3_OK)
		goto out_status;
out:
	return error;
out_status:
	return nfs3_stat_to_errno(status);
}


// Source: nfs3xdr.c
// Lines 1860-1884
