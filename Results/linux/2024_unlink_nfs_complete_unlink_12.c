nfs_complete_unlink(struct dentry *dentry, struct inode *inode)
{
	struct nfs_unlinkdata	*data;

	spin_lock(&dentry->d_lock);
	dentry->d_flags &= ~DCACHE_NFSFS_RENAMED;
	data = dentry->d_fsdata;
	dentry->d_fsdata = NULL;
	spin_unlock(&dentry->d_lock);

	if (NFS_STALE(inode) || !nfs_call_unlink(dentry, inode, data))
		nfs_free_unlinkdata(data);
}


// Source: unlink.c
// Lines 220-232
