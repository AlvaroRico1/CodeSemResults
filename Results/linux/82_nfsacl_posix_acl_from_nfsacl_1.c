posix_acl_from_nfsacl(struct posix_acl *acl)
{
	struct posix_acl_entry *pa, *pe,
	       *group_obj = NULL, *mask = NULL;

	if (!acl)
		return 0;

	sort(acl->a_entries, acl->a_count, sizeof(struct posix_acl_entry),
	     cmp_acl_entry, NULL);

	/* Find the ACL_GROUP_OBJ and ACL_MASK entries. */
	FOREACH_ACL_ENTRY(pa, acl, pe) {
		switch(pa->e_tag) {
			case ACL_USER_OBJ:
				break;
			case ACL_GROUP_OBJ:
				group_obj = pa;
				break;
			case ACL_MASK:
				mask = pa;
				/* fall through */
			case ACL_OTHER:
				break;
		}
	}
	if (acl->a_count == 4 && group_obj && mask &&
	    mask->e_perm == group_obj->e_perm) {
		/* remove bogus ACL_MASK entry */
		memmove(mask, mask+1, (3 - (mask - acl->a_entries)) *
				      sizeof(struct posix_acl_entry));
		acl->a_count = 3;
	}
	return 0;
}


// Source: nfsacl.c
// Lines 219-253
