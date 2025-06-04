static __be32 decode_getattr_args(struct svc_rqst *rqstp,
		struct xdr_stream *xdr, void *argp)
{
	struct cb_getattrargs *args = argp;
	__be32 status;

	status = decode_fh(xdr, &args->fh);
	if (unlikely(status != 0))
		return status;
	return decode_bitmap(xdr, args->bitmap);
}


// Source: callback_xdr.c
// Lines 176-186
