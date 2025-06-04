struct proc_dir_entry *proc_create_data(const char *name, umode_t mode,
		struct proc_dir_entry *parent,
		const struct file_operations *proc_fops, void *data)
{
	struct proc_dir_entry *p;

	BUG_ON(proc_fops == NULL);

	p = proc_create_reg(name, mode, &parent, data);
	if (!p)
		return NULL;
	p->proc_fops = proc_fops;
	return proc_register(parent, p);
}


// Source: generic.c
// Lines 537-550
