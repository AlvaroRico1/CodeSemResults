static int nfs2_xdr_dec_readres(struct rpc_rqst *req, struct xdr_stream *xdr,
				void *data)
{
	struct nfs_pgio_res *result = data;
	enum nfs_stat status;
	int error;

	error = decode_stat(xdr, &status);
	if (unlikely(error))
		goto out;
	result->op_status = status;
	if (status != NFS_OK)
		goto out_default;
	error = decode_fattr(xdr, result->fattr, rpc_rqst_userns(req));
	if (unlikely(error))
		goto out;
	error = decode_nfsdata(xdr, result);
out:
	return error;
out_default:
	return nfs_stat_to_errno(status);
}


// Source: nfs2xdr.c
// Lines 873-894
