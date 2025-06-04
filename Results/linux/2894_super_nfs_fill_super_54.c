void nfs_fill_super(struct super_block *sb, struct nfs_mount_info *mount_info)
{
	struct nfs_parsed_mount_data *data = mount_info->parsed;
	struct nfs_server *server = NFS_SB(sb);

	sb->s_blocksize_bits = 0;
	sb->s_blocksize = 0;
	sb->s_xattr = server->nfs_client->cl_nfs_mod->xattr;
	sb->s_op = server->nfs_client->cl_nfs_mod->sops;
	if (data && data->bsize)
		sb->s_blocksize = nfs_block_size(data->bsize, &sb->s_blocksize_bits);

	if (server->nfs_client->rpc_ops->version != 2) {
		/* The VFS shouldn't apply the umask to mode bits. We will do
		 * so ourselves when necessary.
		 */
		sb->s_flags |= SB_POSIXACL;
		sb->s_time_gran = 1;
		sb->s_export_op = &nfs_export_ops;
	}

 	nfs_initialise_sb(sb);
}


// Source: super.c
// Lines 2366-2388
