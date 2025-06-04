static int nfs3_xdr_dec_read3res(struct rpc_rqst *req, struct xdr_stream *xdr,
				 void *data)
{
	struct nfs_pgio_res *result = data;
	unsigned int pos;
	enum nfs_stat status;
	int error;

	pos = xdr_stream_pos(xdr);
	error = decode_nfsstat3(xdr, &status);
	if (unlikely(error))
		goto out;
	error = decode_post_op_attr(xdr, result->fattr, rpc_rqst_userns(req));
	if (unlikely(error))
		goto out;
	result->op_status = status;
	if (status != NFS3_OK)
		goto out_status;
	result->replen = 4 + ((xdr_stream_pos(xdr) - pos) >> 2);
	error = decode_read3resok(xdr, result);
out:
	return error;
out_status:
	return nfs3_stat_to_errno(status);
}


// Source: nfs3xdr.c
// Lines 1637-1661
