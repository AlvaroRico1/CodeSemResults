int nfs3_set_acl(struct inode *inode, struct posix_acl *acl, int type)
{
	struct posix_acl *alloc = NULL, *dfacl = NULL;
	int status;

	if (S_ISDIR(inode->i_mode)) {
		switch(type) {
		case ACL_TYPE_ACCESS:
			alloc = dfacl = get_acl(inode, ACL_TYPE_DEFAULT);
			if (IS_ERR(alloc))
				goto fail;
			break;

		case ACL_TYPE_DEFAULT:
			dfacl = acl;
			alloc = acl = get_acl(inode, ACL_TYPE_ACCESS);
			if (IS_ERR(alloc))
				goto fail;
			break;
		}
	}

	if (acl == NULL) {
		alloc = acl = posix_acl_from_mode(inode->i_mode, GFP_KERNEL);
		if (IS_ERR(alloc))
			goto fail;
	}
	status = __nfs3_proc_setacls(inode, acl, dfacl);
	posix_acl_release(alloc);
	return status;

fail:
	return PTR_ERR(alloc);
}


// Source: nfs3acl.c
// Lines 254-287
