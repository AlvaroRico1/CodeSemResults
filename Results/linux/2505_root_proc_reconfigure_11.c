static int proc_reconfigure(struct fs_context *fc)
{
	struct super_block *sb = fc->root->d_sb;
	struct pid_namespace *pid = sb->s_fs_info;

	sync_filesystem(sb);

	proc_apply_options(sb, fc, pid, current_user_ns());
	return 0;
}


// Source: root.c
// Lines 145-154
