static struct dentry *proc_pident_instantiate(struct dentry *dentry,
	struct task_struct *task, const void *ptr)
{
	const struct pid_entry *p = ptr;
	struct inode *inode;
	struct proc_inode *ei;

	inode = proc_pid_make_inode(dentry->d_sb, task, p->mode);
	if (!inode)
		return ERR_PTR(-ENOENT);

	ei = PROC_I(inode);
	if (S_ISDIR(inode->i_mode))
		set_nlink(inode, 2);	/* Use getattr to fix if necessary */
	if (p->iop)
		inode->i_op = p->iop;
	if (p->fop)
		inode->i_fop = p->fop;
	ei->op = p->op;
	pid_update_inode(task, inode);
	d_set_d_op(dentry, &pid_dentry_operations);
	return d_splice_alias(inode, dentry);
}


// Source: base.c
// Lines 2452-2474
