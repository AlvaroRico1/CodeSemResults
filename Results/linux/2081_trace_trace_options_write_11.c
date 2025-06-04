trace_options_write(struct file *filp, const char __user *ubuf, size_t cnt,
			 loff_t *ppos)
{
	struct trace_option_dentry *topt = filp->private_data;
	unsigned long val;
	int ret;

	ret = kstrtoul_from_user(ubuf, cnt, 10, &val);
	if (ret)
		return ret;

	if (val != 0 && val != 1)
		return -EINVAL;

	if (!!(topt->flags->val & topt->opt->bit) != val) {
		mutex_lock(&trace_types_lock);
		ret = __set_tracer_option(topt->tr, topt->flags,
					  topt->opt, !val);
		mutex_unlock(&trace_types_lock);
		if (ret)
			return ret;
	}

	*ppos += cnt;

	return cnt;
}


// Source: trace.c
// Lines 7833-7859
