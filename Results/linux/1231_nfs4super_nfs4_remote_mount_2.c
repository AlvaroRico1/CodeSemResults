nfs4_remote_mount(struct file_system_type *fs_type, int flags,
		  const char *dev_name, void *info)
{
	struct nfs_mount_info *mount_info = info;
	struct nfs_server *server;
	struct dentry *mntroot = ERR_PTR(-ENOMEM);

	mount_info->set_security = nfs_set_sb_security;

	/* Get a volume representation */
	server = nfs4_create_server(mount_info, &nfs_v4);
	if (IS_ERR(server)) {
		mntroot = ERR_CAST(server);
		goto out;
	}

	mntroot = nfs_fs_mount_common(server, flags, dev_name, mount_info, &nfs_v4);

out:
	return mntroot;
}


// Source: nfs4super.c
// Lines 108-128
