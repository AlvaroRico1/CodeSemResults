do_setlk(struct file *filp, int cmd, struct file_lock *fl, int is_local)
{
	struct inode *inode = filp->f_mapping->host;
	int status;

	/*
	 * Flush all pending writes before doing anything
	 * with locks..
	 */
	status = nfs_sync_mapping(filp->f_mapping);
	if (status != 0)
		goto out;

	/*
	 * Use local locking if mounted with "-onolock" or with appropriate
	 * "-olocal_lock="
	 */
	if (!is_local)
		status = NFS_PROTO(inode)->lock(filp, cmd, fl);
	else
		status = locks_lock_file_wait(filp, fl);
	if (status < 0)
		goto out;

	/*
	 * Invalidate cache to prevent missing any changes.  If
	 * the file is mapped, clear the page cache as well so
	 * those mappings will be loaded.
	 *
	 * This makes locking act as a cache coherency point.
	 */
	nfs_sync_mapping(filp->f_mapping);
	if (!NFS_PROTO(inode)->have_delegation(inode, FMODE_READ)) {
		nfs_zap_caches(inode);
		if (mapping_mapped(filp->f_mapping))
			nfs_revalidate_mapping(inode, filp->f_mapping);
	}
out:
	return status;
}


// Source: file.c
// Lines 722-761
