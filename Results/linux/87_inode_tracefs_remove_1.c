void tracefs_remove(struct dentry *dentry)
{
	struct dentry *parent;
	int ret;

	if (IS_ERR_OR_NULL(dentry))
		return;

	parent = dentry->d_parent;
	inode_lock(parent->d_inode);
	ret = __tracefs_remove(dentry, parent);
	inode_unlock(parent->d_inode);
	if (!ret)
		simple_release_fs(&tracefs_mount, &tracefs_mount_count);
}


// Source: inode.c
// Lines 533-547
