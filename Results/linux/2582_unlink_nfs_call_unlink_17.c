static int nfs_call_unlink(struct dentry *dentry, struct inode *inode, struct nfs_unlinkdata *data)
{
	struct inode *dir = d_inode(dentry->d_parent);
	struct dentry *alias;

	down_read_non_owner(&NFS_I(dir)->rmdir_sem);
	alias = d_alloc_parallel(dentry->d_parent, &data->args.name, &data->wq);
	if (IS_ERR(alias)) {
		up_read_non_owner(&NFS_I(dir)->rmdir_sem);
		return 0;
	}
	if (!d_in_lookup(alias)) {
		int ret;
		void *devname_garbage = NULL;

		/*
		 * Hey, we raced with lookup... See if we need to transfer
		 * the sillyrename information to the aliased dentry.
		 */
		spin_lock(&alias->d_lock);
		if (d_really_is_positive(alias) &&
		    !(alias->d_flags & DCACHE_NFSFS_RENAMED)) {
			devname_garbage = alias->d_fsdata;
			alias->d_fsdata = data;
			alias->d_flags |= DCACHE_NFSFS_RENAMED;
			ret = 1;
		} else
			ret = 0;
		spin_unlock(&alias->d_lock);
		dput(alias);
		up_read_non_owner(&NFS_I(dir)->rmdir_sem);
		/*
		 * If we'd displaced old cached devname, free it.  At that
		 * point dentry is definitely not a root, so we won't need
		 * that anymore.
		 */
		kfree(devname_garbage);
		return ret;
	}
	data->dentry = alias;
	nfs_do_call_unlink(inode, data);
	return 1;
}


// Source: unlink.c
// Lines 117-159
