static void proc_kill_sb(struct super_block *sb)
{
	struct pid_namespace *ns;

	ns = (struct pid_namespace *)sb->s_fs_info;
	if (ns->proc_self)
		dput(ns->proc_self);
	if (ns->proc_thread_self)
		dput(ns->proc_thread_self);
	kill_anon_super(sb);
	put_pid_ns(ns);
}


// Source: root.c
// Lines 195-206
