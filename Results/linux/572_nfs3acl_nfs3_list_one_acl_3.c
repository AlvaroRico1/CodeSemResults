nfs3_list_one_acl(struct inode *inode, int type, const char *name, void *data,
		size_t size, ssize_t *result)
{
	struct posix_acl *acl;
	char *p = data + *result;

	acl = get_acl(inode, type);
	if (IS_ERR_OR_NULL(acl))
		return 0;

	posix_acl_release(acl);

	*result += strlen(name);
	*result += 1;
	if (!size)
		return 0;
	if (*result > size)
		return -ERANGE;

	strcpy(p, name);
	return 0;
}


// Source: nfs3acl.c
// Lines 296-317
