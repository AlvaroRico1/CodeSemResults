buffer_percent_write(struct file *filp, const char __user *ubuf,
		     size_t cnt, loff_t *ppos)
{
	struct trace_array *tr = filp->private_data;
	unsigned long val;
	int ret;

	ret = kstrtoul_from_user(ubuf, cnt, 10, &val);
	if (ret)
		return ret;

	if (val > 100)
		return -EINVAL;

	if (!val)
		val = 1;

	tr->buffer_percent = val;

	(*ppos)++;

	return cnt;
}


// Source: trace.c
// Lines 8180-8202
