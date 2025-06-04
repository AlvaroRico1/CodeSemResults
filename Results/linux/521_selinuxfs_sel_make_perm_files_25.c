static int sel_make_perm_files(char *objclass, int classvalue,
				struct dentry *dir)
{
	struct selinux_fs_info *fsi = dir->d_sb->s_fs_info;
	int i, rc, nperms;
	char **perms;

	rc = security_get_permissions(fsi->state, objclass, &perms, &nperms);
	if (rc)
		return rc;

	for (i = 0; i < nperms; i++) {
		struct inode *inode;
		struct dentry *dentry;

		rc = -ENOMEM;
		dentry = d_alloc_name(dir, perms[i]);
		if (!dentry)
			goto out;

		rc = -ENOMEM;
		inode = sel_make_inode(dir->d_sb, S_IFREG|S_IRUGO);
		if (!inode) {
			dput(dentry);
			goto out;
		}

		inode->i_fop = &sel_perm_ops;
		/* i+1 since perm values are 1-indexed */
		inode->i_ino = sel_perm_to_ino(classvalue, i + 1);
		d_add(dentry, inode);
	}
	rc = 0;
out:
	for (i = 0; i < nperms; i++)
		kfree(perms[i]);
	kfree(perms);
	return rc;
}


// Source: selinuxfs.c
// Lines 1722-1760
