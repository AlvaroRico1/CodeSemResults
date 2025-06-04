struct proc_dir_entry *proc_create_net_single(const char *name, umode_t mode,
		struct proc_dir_entry *parent,
		int (*show)(struct seq_file *, void *), void *data)
{
	struct proc_dir_entry *p;

	p = proc_create_reg(name, mode, &parent, data);
	if (!p)
		return NULL;
	pde_force_lookup(p);
	p->proc_fops = &proc_net_single_fops;
	p->single_show = show;
	return proc_register(parent, p);
}


// Source: proc_net.c
// Lines 194-207
