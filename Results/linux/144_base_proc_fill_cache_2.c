bool proc_fill_cache(struct file *file, struct dir_context *ctx,
	const char *name, unsigned int len,
	instantiate_t instantiate, struct task_struct *task, const void *ptr)
{
	struct dentry *child, *dir = file->f_path.dentry;
	struct qstr qname = QSTR_INIT(name, len);
	struct inode *inode;
	unsigned type = DT_UNKNOWN;
	ino_t ino = 1;

	child = d_hash_and_lookup(dir, &qname);
	if (!child) {
		DECLARE_WAIT_QUEUE_HEAD_ONSTACK(wq);
		child = d_alloc_parallel(dir, &qname, &wq);
		if (IS_ERR(child))
			goto end_instantiate;
		if (d_in_lookup(child)) {
			struct dentry *res;
			res = instantiate(child, task, ptr);
			d_lookup_done(child);
			if (unlikely(res)) {
				dput(child);
				child = res;
				if (IS_ERR(child))
					goto end_instantiate;
			}
		}


// Source: base.c
// Lines 1882-1908
