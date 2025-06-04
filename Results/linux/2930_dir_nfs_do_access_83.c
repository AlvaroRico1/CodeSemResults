static int nfs_do_access(struct inode *inode, const struct cred *cred, int mask)
{
	struct nfs_access_entry cache;
	bool may_block = (mask & MAY_NOT_BLOCK) == 0;
	int cache_mask;
	int status;

	trace_nfs_access_enter(inode);

	status = nfs_access_get_cached_rcu(inode, cred, &cache);
	if (status != 0)
		status = nfs_access_get_cached(inode, cred, &cache, may_block);
	if (status == 0)
		goto out_cached;

	status = -ECHILD;
	if (!may_block)
		goto out;

	/*
	 * Determine which access bits we want to ask for...
	 */
	cache.mask = NFS_ACCESS_READ | NFS_ACCESS_MODIFY | NFS_ACCESS_EXTEND;
	if (S_ISDIR(inode->i_mode))
		cache.mask |= NFS_ACCESS_DELETE | NFS_ACCESS_LOOKUP;
	else
		cache.mask |= NFS_ACCESS_EXECUTE;
	cache.cred = cred;
	status = NFS_PROTO(inode)->access(inode, &cache);
	if (status != 0) {
		if (status == -ESTALE) {
			nfs_zap_caches(inode);
			if (!S_ISDIR(inode->i_mode))
				set_bit(NFS_INO_STALE, &NFS_I(inode)->flags);
		}
		goto out;
	}
	nfs_access_add_cache(inode, &cache);
out_cached:
	cache_mask = nfs_access_calc_mask(cache.mask, inode->i_mode);
	if ((mask & ~cache_mask & (MAY_READ | MAY_WRITE | MAY_EXEC)) != 0)
		status = -EACCES;
out:
	trace_nfs_access_exit(inode, status);
	return status;
}


// Source: dir.c
// Lines 2466-2511
