cmp_acl_entry(const void *x, const void *y)
{
	const struct posix_acl_entry *a = x, *b = y;

	if (a->e_tag != b->e_tag)
		return a->e_tag - b->e_tag;
	else if ((a->e_tag == ACL_USER) && uid_gt(a->e_uid, b->e_uid))
		return 1;
	else if ((a->e_tag == ACL_USER) && uid_lt(a->e_uid, b->e_uid))
		return -1;
	else if ((a->e_tag == ACL_GROUP) && gid_gt(a->e_gid, b->e_gid))
		return 1;
	else if ((a->e_tag == ACL_GROUP) && gid_lt(a->e_gid, b->e_gid))
		return -1;
	else
		return 0;
}


// Source: nfsacl.c
// Lines 197-213
