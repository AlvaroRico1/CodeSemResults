static int proc_init_fs_context(struct fs_context *fc)
{
	struct proc_fs_context *ctx;

	ctx = kzalloc(sizeof(struct proc_fs_context), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	ctx->pid_ns = get_pid_ns(task_active_pid_ns(current));
	put_user_ns(fc->user_ns);
	fc->user_ns = get_user_ns(ctx->pid_ns->user_ns);
	fc->fs_private = ctx;
	fc->ops = &proc_fs_context_ops;
	return 0;
}


// Source: root.c
// Lines 179-193
