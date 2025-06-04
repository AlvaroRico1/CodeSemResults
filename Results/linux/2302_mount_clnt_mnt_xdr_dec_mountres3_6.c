static int mnt_xdr_dec_mountres3(struct rpc_rqst *req,
				 struct xdr_stream *xdr,
				 void *data)
{
	struct mountres *res = data;
	int status;

	status = decode_fhs_status(xdr, res);
	if (unlikely(status != 0 || res->errno != 0))
		return status;
	status = decode_fhandle3(xdr, res);
	if (unlikely(status != 0)) {
		res->errno = -EBADHANDLE;
		return 0;
	}
	return decode_auth_flavors(xdr, res);
}


// Source: mount_clnt.c
// Lines 454-470
