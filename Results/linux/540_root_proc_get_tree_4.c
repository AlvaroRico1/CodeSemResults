static int proc_get_tree(struct fs_context *fc)
{
	struct proc_fs_context *ctx = fc->fs_private;

	fc->s_fs_info = ctx->pid_ns;
	return vfs_get_super(fc, vfs_get_keyed_super, proc_fill_super);
}


// Source: root.c
// Lines 156-162
