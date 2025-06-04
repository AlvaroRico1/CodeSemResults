int nfsacl_decode(struct xdr_buf *buf, unsigned int base, unsigned int *aclcnt,
		  struct posix_acl **pacl)
{
	struct nfsacl_decode_desc nfsacl_desc = {
		.desc = {
			.elem_size = 12,
			.xcode = pacl ? xdr_nfsace_decode : NULL,
		},
	};
	u32 entries;
	int err;

	if (xdr_decode_word(buf, base, &entries) ||
	    entries > NFS_ACL_MAX_ENTRIES)
		return -EINVAL;
	nfsacl_desc.desc.array_maxlen = entries;
	err = xdr_decode_array2(buf, base + 4, &nfsacl_desc.desc);
	if (err)
		return err;
	if (pacl) {
		if (entries != nfsacl_desc.desc.array_len ||
		    posix_acl_from_nfsacl(nfsacl_desc.acl) != 0) {
			posix_acl_release(nfsacl_desc.acl);
			return -EINVAL;
		}
		*pacl = nfsacl_desc.acl;
	}
	if (aclcnt)
		*aclcnt = entries;
	return 8 + nfsacl_desc.desc.elem_size *
		   nfsacl_desc.desc.array_len;
}


// Source: nfsacl.c
// Lines 265-296
