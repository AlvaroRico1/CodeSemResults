static int decode_fhandle3(struct xdr_stream *xdr, struct mountres *res)
{
	struct nfs_fh *fh = res->fh;
	u32 size;
	__be32 *p;

	p = xdr_inline_decode(xdr, 4);
	if (unlikely(p == NULL))
		return -EIO;

	size = be32_to_cpup(p);
	if (size > NFS3_FHSIZE || size == 0)
		return -EIO;

	p = xdr_inline_decode(xdr, size);
	if (unlikely(p == NULL))
		return -EIO;

	fh->size = size;
	memcpy(fh->data, p, size);
	return 0;
}


// Source: mount_clnt.c
// Lines 397-418
