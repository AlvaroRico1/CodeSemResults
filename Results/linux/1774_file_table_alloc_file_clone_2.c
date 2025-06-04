struct file *alloc_file_clone(struct file *base, int flags,
				const struct file_operations *fops)
{
	struct file *f = alloc_file(&base->f_path, flags, fops);
	if (!IS_ERR(f)) {
		path_get(&f->f_path);
		f->f_mapping = base->f_mapping;
	}
	return f;
}


// Source: file_table.c
// Lines 241-250
