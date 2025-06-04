static int decode_filename_inline(struct xdr_stream *xdr,
				  const char **name, u32 *length)
{
	__be32 *p;
	u32 count;

	p = xdr_inline_decode(xdr, 4);
	if (unlikely(!p))
		return -EIO;
	count = be32_to_cpup(p);
	if (count > NFS3_MAXNAMLEN)
		goto out_nametoolong;
	p = xdr_inline_decode(xdr, count);
	if (unlikely(!p))
		return -EIO;
	*name = (const char *)p;
	*length = count;
	return 0;
out_nametoolong:
	dprintk("NFS: returned filename too long: %u\n", count);
	return -ENAMETOOLONG;
}


// Source: nfs2xdr.c
// Lines 397-418
