static void nfs3_xdr_enc_setacl3args(struct rpc_rqst *req,
				     struct xdr_stream *xdr,
				     const void *data)
{
	const struct nfs3_setaclargs *args = data;
	unsigned int base;
	int error;

	encode_nfs_fh3(xdr, NFS_FH(args->inode));
	encode_uint32(xdr, args->mask);

	base = req->rq_slen;
	if (args->npages != 0)
		xdr_write_pages(xdr, args->pages, 0, args->len);
	else
		xdr_reserve_space(xdr, args->len);

	error = nfsacl_encode(xdr->buf, base, args->inode,
			    (args->mask & NFS_ACL) ?
			    args->acl_access : NULL, 1, 0);
	/* FIXME: this is just broken */
	BUG_ON(error < 0);
	error = nfsacl_encode(xdr->buf, base + error, args->inode,
			    (args->mask & NFS_DFACL) ?
			    args->acl_default : NULL, 1,
			    NFS_ACL_DEFAULT);
	BUG_ON(error < 0);
}


// Source: nfs3xdr.c
// Lines 1340-1367
