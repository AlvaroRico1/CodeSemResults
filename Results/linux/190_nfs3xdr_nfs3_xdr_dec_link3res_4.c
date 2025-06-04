static int nfs3_xdr_dec_link3res(struct rpc_rqst *req, struct xdr_stream *xdr,
				 void *data)
{
	struct user_namespace *userns = rpc_rqst_userns(req);
	struct nfs3_linkres *result = data;
	enum nfs_stat status;
	int error;

	error = decode_nfsstat3(xdr, &status);
	if (unlikely(error))
		goto out;
	error = decode_post_op_attr(xdr, result->fattr, userns);
	if (unlikely(error))
		goto out;
	error = decode_wcc_data(xdr, result->dir_attr, userns);
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
// Lines 1906-1929
