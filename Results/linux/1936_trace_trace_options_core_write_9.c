trace_options_core_write(struct file *filp, const char __user *ubuf, size_t cnt,
			 loff_t *ppos)
{
	void *tr_index = filp->private_data;
	struct trace_array *tr;
	unsigned int index;
	unsigned long val;
	int ret;

	get_tr_index(tr_index, &tr, &index);

	ret = kstrtoul_from_user(ubuf, cnt, 10, &val);
	if (ret)
		return ret;

	if (val != 0 && val != 1)
		return -EINVAL;

	mutex_lock(&trace_types_lock);
	ret = set_tracer_flag(tr, 1 << index, val);
	mutex_unlock(&trace_types_lock);

	if (ret < 0)
		return ret;

	*ppos += cnt;

	return cnt;
}


// Source: trace.c
// Lines 7922-7950
