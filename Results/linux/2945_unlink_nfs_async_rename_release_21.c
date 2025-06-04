static void nfs_async_rename_release(void *calldata)
{
	struct nfs_renamedata	*data = calldata;
	struct super_block *sb = data->old_dir->i_sb;

	if (d_really_is_positive(data->old_dentry))
		nfs_mark_for_revalidate(d_inode(data->old_dentry));

	/* The result of the rename is unknown. Play it safe by
	 * forcing a new lookup */
	if (data->cancelled) {
		spin_lock(&data->old_dir->i_lock);
		nfs_force_lookup_revalidate(data->old_dir);
		spin_unlock(&data->old_dir->i_lock);
		if (data->new_dir != data->old_dir) {
			spin_lock(&data->new_dir->i_lock);
			nfs_force_lookup_revalidate(data->new_dir);
			spin_unlock(&data->new_dir->i_lock);
		}
	}

	dput(data->old_dentry);
	dput(data->new_dentry);
	iput(data->old_dir);
	iput(data->new_dir);
	nfs_sb_deactive(sb);
	put_cred(data->cred);
	kfree(data);
}


// Source: unlink.c
// Lines 280-308
