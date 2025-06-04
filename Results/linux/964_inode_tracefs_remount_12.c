static int tracefs_remount(struct super_block *sb, int *flags, char *data)
{
	int err;
	struct tracefs_fs_info *fsi = sb->s_fs_info;

	sync_filesystem(sb);
	err = tracefs_parse_options(data, &fsi->mount_opts);
	if (err)
		goto fail;

	tracefs_apply_options(sb);

fail:
	return err;
}


// Source: inode.c
// Lines 224-238
