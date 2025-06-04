static void nfs_async_unlink_release(void *calldata)
{
	struct nfs_unlinkdata	*data = calldata;
	struct dentry *dentry = data->dentry;
	struct super_block *sb = dentry->d_sb;

	up_read_non_owner(&NFS_I(d_inode(dentry->d_parent))->rmdir_sem);
	d_lookup_done(dentry);
	nfs_free_unlinkdata(data);
	dput(dentry);
	nfs_sb_deactive(sb);
}


// Source: unlink.c
// Lines 63-74
