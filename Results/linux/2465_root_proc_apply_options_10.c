static void proc_apply_options(struct super_block *s,
			       struct fs_context *fc,
			       struct pid_namespace *pid_ns,
			       struct user_namespace *user_ns)
{
	struct proc_fs_context *ctx = fc->fs_private;

	if (ctx->mask & (1 << Opt_gid))
		pid_ns->pid_gid = make_kgid(user_ns, ctx->gid);
	if (ctx->mask & (1 << Opt_hidepid))
		pid_ns->hide_pid = ctx->hidepid;
}


// Source: root.c
// Lines 85-96
