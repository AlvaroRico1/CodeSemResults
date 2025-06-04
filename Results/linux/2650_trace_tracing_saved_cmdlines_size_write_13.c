tracing_saved_cmdlines_size_write(struct file *filp, const char __user *ubuf,
				  size_t cnt, loff_t *ppos)
{
	unsigned long val;
	int ret;

	ret = kstrtoul_from_user(ubuf, cnt, 10, &val);
	if (ret)
		return ret;

	/* must have at least 1 entry or less than PID_MAX_DEFAULT */
	if (!val || val > PID_MAX_DEFAULT)
		return -EINVAL;

	ret = tracing_resize_saved_cmdlines((unsigned int)val);
	if (ret < 0)
		return ret;

	*ppos += cnt;

	return cnt;
}


// Source: trace.c
// Lines 5175-5196
