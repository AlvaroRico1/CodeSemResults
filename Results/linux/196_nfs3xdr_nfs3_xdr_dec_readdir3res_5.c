static int nfs3_xdr_dec_readdir3res(struct rpc_rqst *req,
				    struct xdr_stream *xdr,
				    void *data)
{
	struct nfs3_readdirres *result = data;
	enum nfs_stat status;
	int error;

	error = decode_nfsstat3(xdr, &status);
	if (unlikely(error))
		goto out;
	if (status != NFS3_OK)
		goto out_default;
	error = decode_readdir3resok(xdr, result, rpc_rqst_userns(req));
out:
	return error;
out_default:
	error = decode_post_op_attr(xdr, result->dir_attr, rpc_rqst_userns(req));
	if (unlikely(error))
		goto out;
	return nfs3_stat_to_errno(status);
}


// Source: nfs3xdr.c
// Lines 2093-2114
