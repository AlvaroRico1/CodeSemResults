static int nfs3_xdr_dec_lookup3res(struct rpc_rqst *req,
				   struct xdr_stream *xdr,
				   void *data)
{
	struct user_namespace *userns = rpc_rqst_userns(req);
	struct nfs3_diropres *result = data;
	enum nfs_stat status;
	int error;

	error = decode_nfsstat3(xdr, &status);
	if (unlikely(error))
		goto out;
	if (status != NFS3_OK)
		goto out_default;
	error = decode_nfs_fh3(xdr, result->fh);
	if (unlikely(error))
		goto out;
	error = decode_post_op_attr(xdr, result->fattr, userns);
	if (unlikely(error))
		goto out;
	error = decode_post_op_attr(xdr, result->dir_attr, userns);
out:
	return error;
out_default:
	error = decode_post_op_attr(xdr, result->dir_attr, userns);
	if (unlikely(error))
		goto out;
	return nfs3_stat_to_errno(status);
}


// Source: nfs3xdr.c
// Lines 1470-1498
