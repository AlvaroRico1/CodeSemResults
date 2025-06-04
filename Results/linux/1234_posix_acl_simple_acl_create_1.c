int simple_acl_create(struct inode *dir, struct inode *inode)
{
	struct posix_acl *default_acl, *acl;
	int error;

	error = posix_acl_create(dir, &inode->i_mode, &default_acl, &acl);
	if (error)
		return error;

	set_cached_acl(inode, ACL_TYPE_DEFAULT, default_acl);
	set_cached_acl(inode, ACL_TYPE_ACCESS, acl);

	if (default_acl)
		posix_acl_release(default_acl);
	if (acl)
		posix_acl_release(acl);
	return 0;
}


// Source: posix_acl.c
// Lines 938-955
