static bool proc_sys_fill_cache(struct file *file,
				struct dir_context *ctx,
				struct ctl_table_header *head,
				struct ctl_table *table)
{
	struct dentry *child, *dir = file->f_path.dentry;
	struct inode *inode;
	struct qstr qname;
	ino_t ino = 0;
	unsigned type = DT_UNKNOWN;

	qname.name = table->procname;
	qname.len  = strlen(table->procname);
	qname.hash = full_name_hash(dir, qname.name, qname.len);

	child = d_lookup(dir, &qname);
	if (!child) {
		DECLARE_WAIT_QUEUE_HEAD_ONSTACK(wq);
		child = d_alloc_parallel(dir, &qname, &wq);
		if (IS_ERR(child))
			return false;
		if (d_in_lookup(child)) {
			struct dentry *res;
			inode = proc_sys_make_inode(dir->d_sb, head, table);
			if (IS_ERR(inode)) {
				d_lookup_done(child);
				dput(child);
				return false;
			}
			d_set_d_op(child, &proc_sys_dentry_operations);
			res = d_splice_alias(inode, child);
			d_lookup_done(child);
			if (unlikely(res)) {
				if (IS_ERR(res)) {
					dput(child);
					return false;
				}
				dput(child);
				child = res;
			}
		}


// Source: proc_sysctl.c
// Lines 688-728
