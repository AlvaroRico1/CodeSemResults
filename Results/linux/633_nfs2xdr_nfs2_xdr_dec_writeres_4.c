static int nfs2_xdr_dec_writeres(struct rpc_rqst *req, struct xdr_stream *xdr,
				 void *data)
{
	struct nfs_pgio_res *result = data;

	/* All NFSv2 writes are "file sync" writes */
	result->verf->committed = NFS_FILE_SYNC;
	return decode_attrstat(xdr, result->fattr, &result->op_status,
			rpc_rqst_userns(req));
}


// Source: nfs2xdr.c
// Lines 896-905
