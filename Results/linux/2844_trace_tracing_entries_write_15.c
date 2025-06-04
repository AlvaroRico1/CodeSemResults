tracing_entries_write(struct file *filp, const char __user *ubuf,
		      size_t cnt, loff_t *ppos)
{
	struct inode *inode = file_inode(filp);
	struct trace_array *tr = inode->i_private;
	unsigned long val;
	int ret;

	ret = kstrtoul_from_user(ubuf, cnt, 10, &val);
	if (ret)
		return ret;

	/* must have at least 1 entry */
	if (!val)
		return -EINVAL;

	/* value is in KB */
	val <<= 10;
	ret = tracing_resize_ring_buffer(tr, val, tracing_get_cpu(inode));
	if (ret < 0)
		return ret;

	*ppos += cnt;

	return cnt;
}


// Source: trace.c
// Lines 6246-6271
