static inline int decode_getacl3resok(struct xdr_stream *xdr,
				      struct nfs3_getaclres *result,
				      struct user_namespace *userns)
{
	struct posix_acl **acl;
	unsigned int *aclcnt;
	size_t hdrlen;
	int error;

	error = decode_post_op_attr(xdr, result->fattr, userns);
	if (unlikely(error))
		goto out;
	error = decode_uint32(xdr, &result->mask);
	if (unlikely(error))
		goto out;
	error = -EINVAL;
	if (result->mask & ~(NFS_ACL|NFS_ACLCNT|NFS_DFACL|NFS_DFACLCNT))
		goto out;

	hdrlen = xdr_stream_pos(xdr);

	acl = NULL;
	if (result->mask & NFS_ACL)
		acl = &result->acl_access;
	aclcnt = NULL;
	if (result->mask & NFS_ACLCNT)
		aclcnt = &result->acl_access_count;
	error = nfsacl_decode(xdr->buf, hdrlen, aclcnt, acl);
	if (unlikely(error <= 0))
		goto out;

	acl = NULL;
	if (result->mask & NFS_DFACL)
		acl = &result->acl_default;
	aclcnt = NULL;
	if (result->mask & NFS_DFACLCNT)
		aclcnt = &result->acl_default_count;
	error = nfsacl_decode(xdr->buf, hdrlen + error, aclcnt, acl);
	if (unlikely(error <= 0))
		return error;
	error = 0;
out:
	return error;
}


// Source: nfs3xdr.c
// Lines 2362-2405
