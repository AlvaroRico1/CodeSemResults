static int nfs_access_get_cached_rcu(struct inode *inode, const struct cred *cred, struct nfs_access_entry *res)
{
	/* Only check the most recently returned cache entry,
	 * but do it without locking.
	 */
	struct nfs_inode *nfsi = NFS_I(inode);
	struct nfs_access_entry *cache;
	int err = -ECHILD;
	struct list_head *lh;

	rcu_read_lock();
	if (nfsi->cache_validity & NFS_INO_INVALID_ACCESS)
		goto out;
	lh = rcu_dereference(nfsi->access_cache_entry_lru.prev);
	cache = list_entry(lh, struct nfs_access_entry, lru);
	if (lh == &nfsi->access_cache_entry_lru ||
	    cred != cache->cred)
		cache = NULL;
	if (cache == NULL)
		goto out;
	if (nfs_check_cache_invalid(inode, NFS_INO_INVALID_ACCESS))
		goto out;
	res->cred = cache->cred;
	res->mask = cache->mask;
	err = 0;
out:
	rcu_read_unlock();
	return err;
}


// Source: dir.c
// Lines 2331-2359
