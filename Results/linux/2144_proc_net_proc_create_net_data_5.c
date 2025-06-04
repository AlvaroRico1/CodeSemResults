struct proc_dir_entry *proc_create_net_data(const char *name, umode_t mode,
		struct proc_dir_entry *parent, const struct seq_operations *ops,
		unsigned int state_size, void *data)
{
	struct proc_dir_entry *p;

	p = proc_create_reg(name, mode, &parent, data);
	if (!p)
		return NULL;
	pde_force_lookup(p);
	p->proc_fops = &proc_net_seq_fops;
	p->seq_ops = ops;
	p->state_size = state_size;
	return proc_register(parent, p);
}


// Source: proc_net.c
// Lines 101-115
