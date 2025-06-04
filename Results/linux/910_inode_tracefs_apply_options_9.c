static int tracefs_apply_options(struct super_block *sb)
{
	struct tracefs_fs_info *fsi = sb->s_fs_info;
	struct inode *inode = sb->s_root->d_inode;
	struct tracefs_mount_opts *opts = &fsi->mount_opts;

	inode->i_mode &= ~S_IALLUGO;
	inode->i_mode |= opts->mode;

	inode->i_uid = opts->uid;
	inode->i_gid = opts->gid;

	return 0;
}


// Source: inode.c
// Lines 209-222
