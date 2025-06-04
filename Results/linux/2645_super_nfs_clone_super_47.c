static void nfs_clone_super(struct super_block *sb,
			    struct nfs_mount_info *mount_info)
{
	const struct super_block *old_sb = mount_info->cloned->sb;
	struct nfs_server *server = NFS_SB(sb);

	sb->s_blocksize_bits = old_sb->s_blocksize_bits;
	sb->s_blocksize = old_sb->s_blocksize;
	sb->s_maxbytes = old_sb->s_maxbytes;
	sb->s_xattr = old_sb->s_xattr;
	sb->s_op = old_sb->s_op;
	sb->s_time_gran = 1;
	sb->s_export_op = old_sb->s_export_op;

	if (server->nfs_client->rpc_ops->version != 2) {
		/* The VFS shouldn't apply the umask to mode bits. We will do
		 * so ourselves when necessary.
		 */
		sb->s_flags |= SB_POSIXACL;
	}

 	nfs_initialise_sb(sb);
}


// Source: super.c
// Lines 2394-2416
