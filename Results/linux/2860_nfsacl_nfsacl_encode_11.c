int nfsacl_encode(struct xdr_buf *buf, unsigned int base, struct inode *inode,
		  struct posix_acl *acl, int encode_entries, int typeflag)
{
	int entries = (acl && acl->a_count) ? max_t(int, acl->a_count, 4) : 0;
	struct nfsacl_encode_desc nfsacl_desc = {
		.desc = {
			.elem_size = 12,
			.array_len = encode_entries ? entries : 0,
			.xcode = xdr_nfsace_encode,
		},
		.acl = acl,
		.typeflag = typeflag,
		.uid = inode->i_uid,
		.gid = inode->i_gid,
	};
	struct nfsacl_simple_acl aclbuf;
	int err;

	if (entries > NFS_ACL_MAX_ENTRIES ||
	    xdr_encode_word(buf, base, entries))
		return -EINVAL;
	if (encode_entries && acl && acl->a_count == 3) {
		struct posix_acl *acl2 = &aclbuf.acl;

		/* Avoid the use of posix_acl_alloc().  nfsacl_encode() is
		 * invoked in contexts where a memory allocation failure is
		 * fatal.  Fortunately this fake ACL is small enough to
		 * construct on the stack. */
		posix_acl_init(acl2, 4);

		/* Insert entries in canonical order: other orders seem
		 to confuse Solaris VxFS. */
		acl2->a_entries[0] = acl->a_entries[0];  /* ACL_USER_OBJ */
		acl2->a_entries[1] = acl->a_entries[1];  /* ACL_GROUP_OBJ */
		acl2->a_entries[2] = acl->a_entries[1];  /* ACL_MASK */
		acl2->a_entries[2].e_tag = ACL_MASK;
		acl2->a_entries[3] = acl->a_entries[2];  /* ACL_OTHER */
		nfsacl_desc.acl = acl2;
	}


// Source: nfsacl.c
// Lines 92-130
