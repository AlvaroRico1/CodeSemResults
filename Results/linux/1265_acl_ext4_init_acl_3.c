ext4_init_acl(handle_t *handle, struct inode *inode, struct inode *dir)
{
	struct posix_acl *default_acl, *acl;
	int error;

	error = posix_acl_create(dir, &inode->i_mode, &default_acl, &acl);
	if (error)
		return error;

	if (default_acl) {
		error = __ext4_set_acl(handle, inode, ACL_TYPE_DEFAULT,
				       default_acl, XATTR_CREATE);
		posix_acl_release(default_acl);
	} else {
		inode->i_default_acl = NULL;
	}
	if (acl) {
		if (!error)
			error = __ext4_set_acl(handle, inode, ACL_TYPE_ACCESS,
					       acl, XATTR_CREATE);
		posix_acl_release(acl);
	} else {
		inode->i_acl = NULL;
	}
	return error;
}


// Source: acl.c
// Lines 275-300
