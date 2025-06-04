xdr_nfsace_encode(struct xdr_array2_desc *desc, void *elem)
{
	struct nfsacl_encode_desc *nfsacl_desc =
		(struct nfsacl_encode_desc *) desc;
	__be32 *p = elem;

	struct posix_acl_entry *entry =
		&nfsacl_desc->acl->a_entries[nfsacl_desc->count++];

	*p++ = htonl(entry->e_tag | nfsacl_desc->typeflag);
	switch(entry->e_tag) {
		case ACL_USER_OBJ:
			*p++ = htonl(from_kuid(&init_user_ns, nfsacl_desc->uid));
			break;
		case ACL_GROUP_OBJ:
			*p++ = htonl(from_kgid(&init_user_ns, nfsacl_desc->gid));
			break;
		case ACL_USER:
			*p++ = htonl(from_kuid(&init_user_ns, entry->e_uid));
			break;
		case ACL_GROUP:
			*p++ = htonl(from_kgid(&init_user_ns, entry->e_gid));
			break;
		default:  /* Solaris depends on that! */
			*p++ = 0;
			break;
	}
	*p++ = htonl(entry->e_perm & S_IRWXO);
	return 0;
}


// Source: nfsacl.c
// Lines 49-78
