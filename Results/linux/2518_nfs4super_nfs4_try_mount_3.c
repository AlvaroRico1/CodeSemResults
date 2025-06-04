struct dentry *nfs4_try_mount(int flags, const char *dev_name,
			      struct nfs_mount_info *mount_info,
			      struct nfs_subversion *nfs_mod)
{
	char *export_path;
	struct vfsmount *root_mnt;
	struct dentry *res;
	struct nfs_parsed_mount_data *data = mount_info->parsed;

	dfprintk(MOUNT, "--> nfs4_try_mount()\n");

	export_path = data->nfs_server.export_path;
	data->nfs_server.export_path = "/";
	root_mnt = nfs_do_root_mount(&nfs4_remote_fs_type, flags, mount_info,
			data->nfs_server.hostname);
	data->nfs_server.export_path = export_path;

	res = nfs_follow_remote_path(root_mnt, export_path);

	dfprintk(MOUNT, "<-- nfs4_try_mount() = %d%s\n",
		 PTR_ERR_OR_ZERO(res),
		 IS_ERR(res) ? " [error]" : "");
	return res;
}


// Source: nfs4super.c
// Lines 238-261
