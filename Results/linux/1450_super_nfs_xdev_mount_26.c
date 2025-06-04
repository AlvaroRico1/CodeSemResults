nfs_xdev_mount(struct file_system_type *fs_type, int flags,
		const char *dev_name, void *raw_data)
{
	struct nfs_clone_mount *data = raw_data;
	struct nfs_mount_info mount_info = {
		.fill_super = nfs_clone_super,
		.set_security = nfs_clone_sb_security,
		.cloned = data,
	};
	struct nfs_server *server;
	struct dentry *mntroot = ERR_PTR(-ENOMEM);
	struct nfs_subversion *nfs_mod = NFS_SB(data->sb)->nfs_client->cl_nfs_mod;

	dprintk("--> nfs_xdev_mount()\n");

	mount_info.mntfh = mount_info.cloned->fh;

	/* create a new volume representation */
	server = nfs_mod->rpc_ops->clone_server(NFS_SB(data->sb), data->fh, data->fattr, data->authflavor);

	if (IS_ERR(server))
		mntroot = ERR_CAST(server);
	else
		mntroot = nfs_fs_mount_common(server, flags,
				dev_name, &mount_info, nfs_mod);

	dprintk("<-- nfs_xdev_mount() = %ld\n",
			IS_ERR(mntroot) ? PTR_ERR(mntroot) : 0L);
	return mntroot;
}


// Source: super.c
// Lines 2772-2801
