static int mnt_xdr_dec_mountres(struct rpc_rqst *req,
				struct xdr_stream *xdr,
				void *data)
{
	struct mountres *res = data;
	int status;

	status = decode_status(xdr, res);
	if (unlikely(status != 0 || res->errno != 0))
		return status;
	return decode_fhandle(xdr, res);
}


// Source: mount_clnt.c
// Lines 361-372
