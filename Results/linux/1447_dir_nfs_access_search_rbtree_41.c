static struct nfs_access_entry *nfs_access_search_rbtree(struct inode *inode, const struct cred *cred)
{
	struct rb_node *n = NFS_I(inode)->access_cache.rb_node;

	while (n != NULL) {
		struct nfs_access_entry *entry =
			rb_entry(n, struct nfs_access_entry, rb_node);
		int cmp = cred_fscmp(cred, entry->cred);

		if (cmp < 0)
			n = n->rb_left;
		else if (cmp > 0)
			n = n->rb_right;
		else
			return entry;
	}
	return NULL;
}


// Source: dir.c
// Lines 2269-2286
